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
#include <proto/resp.hpp>
#include <server/session.hpp>

using mydss::db::Inst;
using mydss::err::ErrnoStr;
using mydss::err::kEof;
using mydss::err::Status;
using mydss::proto::ErrorPiece;
using mydss::proto::Piece;
using mydss::proto::Req;
using mydss::proto::Resp;
using mydss::util::Slice;
using std::make_shared;
using std::shared_ptr;
using std::string;
using std::vector;
using std::placeholders::_1;
using std::placeholders::_2;

namespace mydss::server {

static constexpr size_t kRecvBufSize = 2048;

void Session::Start() {
  auto slice = Slice(kRecvBufSize);
  conn_->AsyncRecv(slice,
                   bind(&Session::OnRecv, shared_from_this(), slice, _1, _2));
}

void Session::Send(shared_ptr<Piece> piece, bool close) {
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

  vector<Req> reqs;
  status = session->parser_.Parse(slice.data(), nbytes, reqs);
  if (status.error()) {
    auto resp = make_shared<ErrorPiece>(status.msg());
    session->Send(resp, true);
    return;
  }

  for (auto& req : reqs) {
    req.set_client(session->client());

    Resp resp;
    Inst::GetInst()->Handle(req, resp);
    assert(resp.piece());

    if (resp.close()) {
      session->Send(resp.piece(), true);
      return;
    }
    session->Send(resp.piece());
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
