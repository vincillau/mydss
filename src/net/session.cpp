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

#include <db/inst.hpp>
#include <err/errno.hpp>
#include <net/session.hpp>
#include <proto/resp.hpp>

using mydss::db::Inst;
using mydss::err::ErrnoStr;
using mydss::err::kOk;
using mydss::proto::ErrorPiece;
using mydss::proto::Piece;
using mydss::proto::Req;
using mydss::proto::Resp;
using mydss::util::Buf;
using std::make_shared;
using std::shared_ptr;
using std::string;
using std::vector;
using std::placeholders::_1;
using std::placeholders::_2;

namespace mydss::net {
static constexpr size_t kRecvBufSize = 2048;

void Session::Start() {
  auto buf = shared_ptr<char[]>(new char[kRecvBufSize]);
  conn_->AsyncRecv(Buf(buf.get(), kRecvBufSize),
                   bind(&Session::OnRecv, shared_from_this(), buf, _1, _2));
}

void Session::Send(shared_ptr<Piece> piece, bool close) {
  size_t buf_size = piece->Size();
  auto buf = shared_ptr<char[]>(new char[buf_size]);

  size_t nbytes = piece->Serialize(buf.get(), buf_size);
  assert(nbytes == buf_size);

  conn_->AsyncSend(
      Buf(buf.get(), buf_size),
      bind(&Session::OnSend, shared_from_this(), buf, close, _1, _2));
}

void Session::OnRecv(shared_ptr<Session> session, shared_ptr<char[]> buf,
                     int err, int nbytes) {
  if (err != kOk) {
    SPDLOG_DEBUG("receive data failed, errno={}, reason='{}'", errno,
                 ErrnoStr());
    session->conn_->Close();
    return;
  }

  vector<Req> reqs;
  auto status = session->parser_.Parse(buf.get(), nbytes, reqs);
  if (status.error()) {
    auto resp = make_shared<ErrorPiece>(status.msg());
    session->Send(resp, true);
    return;
  }

  for (const auto& req : reqs) {
    Resp resp;
    Inst::GetInst()->Handle(req, resp);
    assert(resp.piece());
    session->Send(resp.piece());
  }

  session->conn_->AsyncRecv(Buf(buf.get(), kRecvBufSize),
                            bind(&Session::OnRecv, session, buf, _1, _2));
}

void Session::OnSend(shared_ptr<Session> session, shared_ptr<char[]> buf,
                     bool close, int err, int nbytes) {
  if (err != kOk) {
    SPDLOG_DEBUG("send data failed, errno={}, reason='{}'", errno, ErrnoStr());
    session->conn_->Close();
    return;
  }

  if (close) {
    SPDLOG_DEBUG("close the connection");
    session->conn_->Close();
  }
}

}  // namespace mydss::net
