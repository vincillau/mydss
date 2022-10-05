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
#include <net/conn.hpp>

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

  // 开始监听读事件
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

void Conn::AsyncSend(const Buf& buf, SendHandler handler) {
  assert(handler);

  if (send_queue_.size() > 0) {
    // 套接字写缓冲区已满，直接追加到发送队列
    send_queue_.push_back({buf, std::move(handler)});
  }

  // 尝试发送
  int nbytes = write(sock_, buf.data(), buf.len());
  if (nbytes == buf.len()) {
    // 发送成功
    handler(kOk, nbytes);
    return;
  }
  if (errno != EAGAIN) {
    // 发送失败
    // TODO(Vincil Lau): 暂时使用 errno 作为错误码
    handler(errno, nbytes);
    return;
  }

  // 套接字写缓冲区已满，开始监听写事件
  loop_->SetWriteHandler(sock_, bind(&Conn::OnSend, shared_from_this()));
  send_queue_.push_back({buf, std::move(handler)});
}

void Conn::OnRecv(shared_ptr<Conn> conn) {
  assert(conn->recv_handler_);
  assert(conn->recv_buf_.data() != nullptr);

  int nbytes = read(conn->sock_, conn->recv_buf_.data(), conn->recv_buf_.len());
  int code = kOk;
  code = (nbytes == 0) ? kEof : code;
  code = (nbytes == -1) ? kUnknown : code;

  // 每次触发读事件后移除 recv_handler_
  auto handler = std::move(conn->recv_handler_);
  handler(code, nbytes);
}

void Conn::OnSend(shared_ptr<Conn> conn) {
  assert(conn->send_queue_.size() > 0);

  while (conn->send_queue_.size() > 0) {
    const Buf& buf = conn->send_queue_.front().first;
    const SendHandler& handler = std::move(conn->send_queue_.front().second);

    int nbytes = write(conn->sock_, buf.data(), buf.len());
    if (nbytes == buf.len()) {
      handler(kOk, nbytes);
      conn->send_queue_.pop_front();
      continue;
    }
    if (errno != EAGAIN) {
      // TODO(Vincil Lau): 暂时使用 errno 作为错误码
      handler(errno, nbytes);
      conn->send_queue_.pop_front();
      continue;
    }
    // 尚未发送完毕，等待下一个写事件
    return;
  }

  // 发送完毕，停止监听写事件
  conn->loop_->SetWriteHandler(conn->sock_, {});
}

}  // namespace mydss::net
