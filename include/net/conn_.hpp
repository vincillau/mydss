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
#include <util/buf.hpp>

#include "addr.hpp"
#include "loop.hpp"

namespace mydss::net {

class Conn : public std::enable_shared_from_this<Conn> {
 public:
  using RecvHandler =
      std::function<void(int /* MyDSS 错误码 */, int /* 接收的字节数 */)>;
  using SendHandler =
      std::function<void(int /* MyDSS 错误码 */, int /* 发送的字节数 */)>;

  // 保证所有的 Conn 对象都由 std::shared_ptr 持有
  [[nodiscard]] static auto New(std::shared_ptr<Loop> loop) {
    return std::shared_ptr<Conn>(new Conn(loop));
  }

  // Conn 对象在销毁前必须被关闭，防止文件描述符泄漏
  ~Conn() { assert(sock_ == -1); }

  // 异步接收数据，将接收到的数据存储到 buf，并调用 handler
  void AsyncRecv(util::Buf buf, RecvHandler handler);

  void AsyncSend(const util::Buf& buf, SendHandler handler);

  // 在建立连接时调用，将设置连接套接字 sock 和远程的地址
  void Connect(int sock, Addr remote);

  // 关闭连接
  void Close();

 private:
  explicit Conn(std::shared_ptr<Loop> loop) : loop_(loop), sock_(-1) {}

  // 处理读事件，读取数据到 buf，并调用 handler
  static void OnRecv(std::shared_ptr<Conn> conn);

  static void OnSend(std::shared_ptr<Conn> conn);

 private:
  // 监听 Conn 读写事件的事件循环
  std::shared_ptr<Loop> loop_;
  // 连接套接字
  int sock_;
  // 远程的地址
  Addr remote_;
  // 存储接收到的数据的缓冲区
  util::Buf recv_buf_;
  // 处理读事件的 handler
  RecvHandler recv_handler_;
  std::list<SendHandler> send_handlers_;
};

}  // namespace mydss::net

#endif  // MYDSS_INCLUDE_NET_CONN_HPP_
