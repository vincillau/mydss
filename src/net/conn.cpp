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

#include <unistd.h>

#include <err/code.hpp>
#include <err/errno.hpp>
#include <net/conn.hpp>

using mydss::err::ErrnoStr;
using mydss::err::kEof;
using mydss::err::Status;
using mydss::util::Slice;
using std::shared_ptr;

namespace mydss::net {

void Conn::Close() {
  assert(sock_ != -1);

  // 清空所有尚未完成的读写请求
  recv_reqs_.clear();
  send_reqs_.clear();

  if (loop_ != nullptr) {
    auto status = loop_->Remove(sock_);
    assert(status.ok());
    loop_ = nullptr;
  }

  close(sock_);
  sock_ = -1;
}

void Conn::AsyncRecv(Slice slice, RecvHandler handler) {
  if (recv_reqs_.size() > 0) {
    recv_reqs_.emplace_back(slice, std::move(handler));
    return;
  }

  auto nbytes = read(sock_, slice.data(), slice.size());
  if (nbytes == 0) {
    handler({kEof, "end of file"}, 0);
    return;
  } else if (nbytes > 0) {
    handler(Status::Ok(), nbytes);
    return;
  }

  // nbytes == -1
  if (errno != EAGAIN) {
    handler({errno, ErrnoStr()}, 0);
    return;
  }

  // 监听可读事件
  auto status = loop_->SetInEvent(sock_, bind(OnRecv, shared_from_this()));
  if (status.error()) {
    handler(std::move(status), 0);
  }
  recv_reqs_.emplace_back(slice, std::move(handler));
}

void Conn::AsyncSend(Slice slice, SendHandler handler) {
  if (send_reqs_.size() > 0) {
    send_reqs_.emplace_back(slice, std::move(handler));
    return;
  }

  // 允许 slice 为空，一般用于检测发送队列是否发送完成
  if (slice.empty()) {
    handler(Status::Ok());
    return;
  }

  auto nbytes = write(sock_, slice.data(), slice.size());
  assert(nbytes != 0);
  if (nbytes == slice.size()) {
    handler(Status::Ok());
    return;
  } else if (nbytes > 0) {
    Slice unsent(slice, nbytes, slice.size());
    send_reqs_.emplace_back(slice, std::move(handler));
    // 监听可写事件
    auto status = loop_->SetOutEvent(sock_, bind(OnSend, shared_from_this()));
    if (status.error()) {
      handler(std::move(status));
    }
    return;
  }

  // nbytes == -1
  if (errno != EAGAIN) {
    handler({errno, ErrnoStr()});
    return;
  }

  send_reqs_.emplace_back(slice, std::move(handler));
  // 监听可写事件
  auto status = loop_->SetOutEvent(sock_, bind(OnSend, shared_from_this()));
  if (status.error()) {
    handler(std::move(status));
  }
}

void Conn::OnRecv(shared_ptr<Conn> conn) {
  assert(conn->recv_reqs_.size() > 0);

  while (conn->recv_reqs_.size() > 0) {
    auto req = std::move(conn->recv_reqs_.front());
    conn->recv_reqs_.pop_front();

    auto nbytes = read(conn->sock_, req.slice().data(), req.slice().size());
    if (nbytes == 0) {
      req.handler()({kEof, "end of file"}, 0);
      return;
    } else if (nbytes == -1) {
      if (errno != EAGAIN) {
        req.handler()({errno, ErrnoStr()}, 0);
      }
      return;
    }

    req.handler()(Status::Ok(), nbytes);
  }

  // 当连接在回调中关闭时，conn->loop_ 为 nullptr
  if (conn->loop_ != nullptr) {
    // 停止监听可读事件
    auto status = conn->loop_->SetInEvent(conn->sock_, {});
    assert(status.ok());
  }
}

void Conn::OnSend(shared_ptr<Conn> conn) {
  assert(conn->send_reqs_.size() > 0);

  while (conn->send_reqs_.size() > 0) {
    auto req = std::move(conn->send_reqs_.front());
    conn->send_reqs_.pop_front();

    auto nbytes = write(conn->sock_, req.slice().data(), req.slice().size());
    assert(nbytes != 0);
    if (nbytes == req.slice().size()) {
      req.handler()(Status::Ok());
      conn->send_reqs_.pop_front();
      continue;
    } else if (nbytes > 0) {
      Slice unsent(req.slice(), nbytes, req.slice().size());
      req.set_slice(unsent);
      return;
    } else {
      req.handler()({errno, ErrnoStr()});
    }
  }

  // 停止监听可写事件
  auto status = conn->loop_->SetOutEvent(conn->sock_, {});
  assert(status.ok());
}

}  // namespace mydss::net
