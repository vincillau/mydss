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

#include <instance.hpp>
#include <net/conn.hpp>
#include <string>

using asio::buffer;
using asio::const_buffer;
using asio::error_code;
using asio::error::eof;
using std::bind;
using std::make_shared;
using std::shared_ptr;
using std::string;
using std::vector;
using std::placeholders::_1;
using std::placeholders::_2;

namespace mydss {

void Conn::Send(shared_ptr<SendReq> send_req) {
  SPDLOG_DEBUG("try to send {} bytes to {}", send_req->data().size(),
               GetRemoteStr());
  sock_->async_send(
      const_buffer(send_req->data().c_str(), send_req->data().size()),
      bind(&Conn::AfterSend, shared_from_this(), send_req, _1, _2));
}

void Conn::Recv(size_t buf_size) {
  SPDLOG_DEBUG("try to receive from {}, buf_size={}", GetRemoteStr(), buf_size);

  auto buf = make_shared<char*>(new char[buf_size]);
  sock_->async_receive(buffer(*buf, buf_size),
                       bind(&Conn::OnRecv, shared_from_this(), buf, _1, _2));
}

void Conn::HandleRecvErr(const error_code& err) {
  if (err == eof) {
    SPDLOG_DEBUG(
        "recv error from {}: `({}) {}`, remote closed the "
        "connection",
        GetRemoteStr(), err, err.message());

  } else {
    SPDLOG_ERROR(
        "recv error from {}: `({}) {}`, close the "
        "connection",
        GetRemoteStr(), err, err.message());
  }

  // 出错后关闭连接
  sock_->close();
}

void Conn::OnRecv(shared_ptr<Conn> conn, shared_ptr<char*> buf,
                  const error_code& err, size_t nrecv) {
  if (err) {
    conn->HandleRecvErr(err);
    return;
  }
  SPDLOG_DEBUG("received {} bytes from {}", nrecv, conn->GetRemoteStr());

  vector<Req> reqs;
  Status status = conn->parser_.Parse(*buf, nrecv, reqs);
  if (status.error()) {
    SPDLOG_WARN("parse error: `{}` for '{}'", status, string(*buf, nrecv));
    conn->SendError(status.msg());
    return;
  }

  for (const auto& req : reqs) {
    SPDLOG_DEBUG("handle req: {}", req);

    auto inst = Instance::GetInstance();
    std::shared_ptr<Piece> result;
    inst->Handle(req, result);
    conn->SendResult(*result);
  }
}

void Conn::AfterSend(shared_ptr<Conn> conn, shared_ptr<SendReq> send_req,
                     const error_code& err, std::size_t nsend) {
  if (err) {
    SPDLOG_ERROR("send to {} error: `({}) {}`", conn->GetRemoteStr(), err,
                 err.message());
    // 出错后关闭连接
    conn->sock_->close();
    return;
  }

  SPDLOG_DEBUG("sent {} bytes to {}", nsend, conn->GetRemoteStr());

  switch (send_req->action()) {
    case SendReq::Action::kDoNothing:
      break;
    case SendReq::Action::kRecv:
      conn->Recv(kBufSize);
      break;
    case SendReq::Action::kClose:
      SPDLOG_DEBUG("close connection from {}", conn->GetRemoteStr());
      conn->sock_->close();
      break;
  }
}

}  // namespace mydss
