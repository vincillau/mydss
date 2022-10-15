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

#ifndef MYDSS_INCLUDE_NET_ACCEPTOR_HPP_
#define MYDSS_INCLUDE_NET_ACCEPTOR_HPP_

#include <functional>
#include <list>

#include "conn.hpp"
#include "end_point.hpp"
#include "loop.hpp"

namespace mydss::net {

class Acceptor : public std::enable_shared_from_this<Acceptor> {
 public:
  // 接受连接事件的 handler
  using AcceptHandler = std::function<void(err::Status)>;

  // 保证所有的 Acceptor 对象都由 std::shared_ptr 持有
  [[nodiscard]] static auto New(std::shared_ptr<Loop> loop, EndPoint ep) {
    return std::shared_ptr<Acceptor>(new Acceptor(loop, std::move(ep)));
  }

  // 开始接受连接
  // backlog 传递给 listen 系统调用
  [[nodiscard]] err::Status Start(int backlog);

  // 异步接受连接
  void AsyncAccept(std::shared_ptr<Conn> conn, AcceptHandler handler);

 private:
  // 接收连接的请求
  class Req;

  // 构造 Acceptor 对象
  explicit Acceptor(std::shared_ptr<Loop> loop, EndPoint ep)
      : loop_(loop), listen_fd_(-1), ep_(std::move(ep)) {}

  // 接受连接
  err::Status Accept(std::shared_ptr<Conn> conn);

  // 处理可以建立连接的事件
  static void OnAccept(std::shared_ptr<Acceptor> acceptor);

 private:
  std::shared_ptr<Loop> loop_;  // 监听 listen_fd_ 的事件循环
  int listen_fd_;               // 监听套接字
  EndPoint ep_;                 // 绑定的端点
  std::list<Req> reqs_;         // 建立连接的请求
};

class Acceptor::Req {
 public:
  Req() = default;
  Req(std::shared_ptr<Conn> conn, AcceptHandler handler)
      : conn_(conn), handler_(std::move(handler)) {}

  [[nodiscard]] auto& conn() { return conn_; }
  [[nodiscard]] const auto& handler() const { return handler_; }

 private:
  std::shared_ptr<Conn> conn_;
  AcceptHandler handler_;
};

}  // namespace mydss::net

#endif  // MYDSS_INCLUDE_NET_ACCEPTOR_HPP_
