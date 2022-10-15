// Copyright 2022 Vincil Lau
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <spdlog/spdlog.h>

#include <db/inst.hpp>
#include <err/errno.hpp>
#include <server/session.hpp>

using mydss::db::Inst;
using mydss::err::ErrnoStr;
using mydss::err::kEof;
using mydss::err::Status;
using mydss::module::Ctx;
using mydss::module::ErrorPiece;
using mydss::module::Piece;
using mydss::net::Conn;
using mydss::util::Slice;
using std::make_shared;
using std::shared_ptr;
using std::string;
using std::unordered_map;
using std::vector;
using std::placeholders::_1;
using std::placeholders::_2;

namespace mydss::server {

static constexpr size_t kRecvBufSize = 2048;

uint64_t Session::next_id_ = 1;
unordered_map<uint64_t, shared_ptr<Session>> Session::map_;

Session::Session(shared_ptr<Conn> conn) : conn_(conn), id_(next_id_) {
  next_id_++;
}

void Session::Start() {
  map_[id_] = shared_from_this();
  auto slice = Slice(kRecvBufSize);
  conn_->AsyncRecv(slice,
                   bind(&Session::OnRecv, shared_from_this(), slice, _1, _2));
}

void Session::Send(shared_ptr<Piece> piece, bool close) {
  if (piece == nullptr) {
    Slice slice;
    conn_->AsyncSend(
        slice, bind(&Session::OnSend, shared_from_this(), slice, close, _1));
    return;
  }

  size_t size = piece->Size();
  auto slice = Slice(size);

  size_t nbytes = piece->Serialize(slice.data(), size);
  assert(nbytes == size);

  conn_->AsyncSend(
      slice, bind(&Session::OnSend, shared_from_this(), slice, close, _1));
}

void Session::OnRecv(shared_ptr<Session> session, Slice slice, Status status,
                     int nbytes) {
  if (status.code() == kEof) {
    SPDLOG_DEBUG("receive data failed, errno={}, reason='{}'", errno,
                 ErrnoStr());
    session->conn_->Close();
    return;
  }
  if (status.error()) {
    SPDLOG_CRITICAL("{}", status.ToString());
    abort();
  }

  vector<vector<string>> reqs;
  status = session->parser_.Parse(slice.data(), nbytes, reqs);
  if (status.error()) {
    auto resp = make_shared<ErrorPiece>(status.msg());
    session->Send(resp, true);
    return;
  }

  for (auto& req : reqs) {
    Ctx ctx(session->id_);
    Inst::GetInst()->Handle(ctx, req);
    if (session->conn_->closed()) {
      return;
    }
  }

  session->conn_->AsyncRecv(slice,
                            bind(&Session::OnRecv, session, slice, _1, _2));
}

void Session::OnSend(std::shared_ptr<Session> session, util::Slice slice,
                     bool close, err::Status status) {
  if (status.error()) {
    SPDLOG_DEBUG("send data failed, errno={}, reason='{}'", errno, ErrnoStr());
    session->conn_->Close();
    return;
  }

  if (close) {
    SPDLOG_DEBUG("close the connection");
    session->conn_->Close();
  }
}

}  // namespace mydss::server
