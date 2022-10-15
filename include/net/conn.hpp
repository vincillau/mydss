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

#ifndef MYDSS_INCLUDE_NET_CONN_HPP_
#define MYDSS_INCLUDE_NET_CONN_HPP_

#include <cassert>
#include <list>
#include <util/slice.hpp>

#include "end_point.hpp"
#include "loop.hpp"

namespace mydss::net {

class Conn : public std::enable_shared_from_this<Conn> {
 public:
  using RecvHandler = std::function<void(err::Status, int64_t)>;
  using SendHandler = std::function<void(err::Status)>;

  // 保证所有的 Conn 对象都由 std::shared_ptr 持有
  [[nodiscard]] static auto New() { return std::shared_ptr<Conn>(new Conn()); }

  // Conn 对象在销毁前必须被关闭，防止文件描述符泄漏
  ~Conn() { assert(sock_ == -1); }

  [[nodiscard]] bool closed() const { return sock_ == -1; }

  void Attach(std::shared_ptr<Loop> loop) {
    assert(loop_ == nullptr);
    assert(!loop->Contains(sock_));
    loop_ = loop;
  }

  // 异步接收数据
  void AsyncRecv(util::Slice slice, RecvHandler handler);

  // 发送 slice 中的数据，在发送完成后调用 handler
  void AsyncSend(util::Slice slice, SendHandler handler);

  // 在建立连接时调用，将设置连接套接字 sock 和远程的地址
  void Connect(int sock, EndPoint remote) {
    sock_ = sock;
    remote_ = std::move(remote);
  }

  // 关闭连接
  void Close();

 private:
  class RecvReq;
  class SendReq;

  Conn() : sock_(-1) {}

  // 套接字可读事件的处理函数
  static void OnRecv(std::shared_ptr<Conn> conn);
  // 套接字可写事件的处理函数
  static void OnSend(std::shared_ptr<Conn> conn);

 private:
  // 监听 Conn 读写事件的事件循环
  std::shared_ptr<Loop> loop_;
  // 连接套接字
  int sock_;
  // 远程的端点
  EndPoint remote_;
  // 接收数据的请求队列
  std::list<RecvReq> recv_reqs_;
  // 发送数据的请求队列
  std::list<SendReq> send_reqs_;
};

class Conn::RecvReq {
 public:
  RecvReq(const util::Slice& slice, Conn::RecvHandler handler)
      : slice_(slice), handler_(std::move(handler)) {}

  [[nodiscard]] const auto& slice() const { return slice_; }
  [[nodiscard]] const auto& handler() const { return handler_; }

 private:
  util::Slice slice_;
  Conn::RecvHandler handler_;
};

class Conn::SendReq {
 public:
  SendReq(const util::Slice& slice, Conn::SendHandler handler)
      : slice_(slice), handler_(std::move(handler)) {}

  [[nodiscard]] const auto& slice() const { return slice_; }
  void set_slice(util::Slice slice) { slice_ = slice; }
  [[nodiscard]] const auto& handler() const { return handler_; }

 private:
  util::Slice slice_;
  Conn::SendHandler handler_;
};

}  // namespace mydss::net

#endif  // MYDSS_INCLUDE_NET_CONN_HPP_
