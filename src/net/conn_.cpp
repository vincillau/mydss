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
#include <unistd.h>

#include <err/code.hpp>
#include <net/conn_.hpp>

using mydss::err::kEof;
using mydss::err::kOk;
using mydss::err::kUnknown;
using mydss::util::Buf;
using std::bind;
using std::shared_ptr;

namespace mydss::net {

void Conn::Connect(int sock, Addr remote) {
  sock_ = sock;
  remote_ = std::move(remote);

  loop_->Add(sock_, bind(&Conn::OnRecv, shared_from_this()));
}

void Conn::Close() {
  assert(sock_ != -1);

  loop_->Remove(sock_);
  close(sock_);
  sock_ = -1;
}

void Conn::AsyncRecv(Buf buf, RecvHandler handler) {
  assert(handler);
  assert(!recv_handler_);

  int nbytes = read(sock_, buf.data(), buf.len());
  if (nbytes > 0) {
    handler(kOk, nbytes);
    return;
  }
  if (nbytes == 0) {
    handler(kEof, nbytes);
    return;
  }
  if (errno != EAGAIN) {
    // TODO(Vincil Lau): 暂时使用 errno 作为错误码
    handler(errno, nbytes);
    return;
  }

  recv_handler_ = std::move(handler);
  recv_buf_ = buf;
}

void Conn::AsyncSend(const Buf& buf, SendHandler handler) {}

void Conn::OnRecv(shared_ptr<Conn> conn) {
  assert(conn->recv_handler_);
  assert(conn->recv_buf_.data() != nullptr);

  int nbytes = read(conn->sock_, conn->recv_buf_.data(), conn->recv_buf_.len());
  int code = kOk;
  if (nbytes == 0) {
    code = kEof;
  } else if (nbytes == -1) {
    code = kUnknown;
  }

  // 每次触发读事件后移除 recv_handler_
  auto handler = std::move(conn->recv_handler_);
  handler(code, nbytes);
}

void Conn::OnSend(shared_ptr<Conn> conn) {}

}  // namespace mydss::net
