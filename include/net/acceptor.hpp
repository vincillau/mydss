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

#include "addr.hpp"
#include "conn_.hpp"
#include "loop.hpp"

namespace mydss::net {

class Acceptor : public std::enable_shared_from_this<Acceptor> {
 public:
  using AcceptHandler = std::function<void()>;

  static constexpr int kBackLog = 512;

  // 保证所有的 Acceptor 对象都由 std::shared_ptr 持有
  [[nodiscard]] static auto New(std::shared_ptr<Loop> loop, Addr addr) {
    return std::shared_ptr<Acceptor>(new Acceptor(loop, std::move(addr)));
  }

  void Start() {
    Bind();
    Listen();
  }

  // 异步接收连接
  void AsyncAccept(std::shared_ptr<Conn> conn, AcceptHandler handler);

 private:
  // 创建监听 addr 的 Acceptor
  Acceptor(std::shared_ptr<Loop> loop, Addr addr);

  void Bind();
  void Listen();
  bool Accept(std::shared_ptr<Conn> conn);

  // 处理可以建立连接的事件
  static void OnAccept(std::shared_ptr<Acceptor> acceptor);

 private:
  std::shared_ptr<Loop> loop_;
  Addr addr_;
  int listen_fd_;
  AcceptHandler handler_;
  std::shared_ptr<Conn> conn_;
};

}  // namespace mydss::net

#endif  // MYDSS_INCLUDE_NET_ACCEPTOR_HPP_
