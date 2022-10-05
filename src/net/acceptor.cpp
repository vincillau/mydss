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

#include <arpa/inet.h>
#include <spdlog/spdlog.h>
#include <sys/socket.h>

#include <cstdlib>
#include <cstring>
#include <err/errno_str.hpp>
#include <net/acceptor.hpp>
#include <util/fd.hpp>

using mydss::err::ErrnoStr;
using mydss::util::FdSetNonBlock;
using std::shared_ptr;

namespace mydss::net {

Acceptor::Acceptor(std::shared_ptr<Loop> loop, Addr addr)
    : loop_(loop),
      addr_(std::move(addr)),
      listen_fd_(socket(AF_INET, SOCK_STREAM, 0)) {
  // 创建监听套接字
  if (listen_fd_ == -1) {
    SPDLOG_CRITICAL("create socket error: {}", ErrnoStr());
    exit(EXIT_FAILURE);
  }

  // 设置非阻塞
  bool ok = FdSetNonBlock(listen_fd_);
  if (!ok) {
    SPDLOG_CRITICAL("FdSetNonblock error: {}", ErrnoStr());
    exit(EXIT_FAILURE);
  }

  // 设置端口复用
  int opt = 1;
  int ret = setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  if (ret == -1) {
    SPDLOG_CRITICAL("setsockopt error: {}", ErrnoStr());
    exit(EXIT_FAILURE);
  }
}

void Acceptor::Bind() {
  // 转换地址
  sockaddr_in sock_addr;
  bool ok = addr_.ToSockAddrIn(sock_addr);
  if (!ok) {
    SPDLOG_CRITICAL("inet_pton error: {}", ErrnoStr());
    exit(EXIT_FAILURE);
  }

  // 绑定地址
  int ret =
      bind(listen_fd_, reinterpret_cast<const struct sockaddr*>(&sock_addr),
           sizeof(sock_addr));
  if (ret == -1) {
    SPDLOG_CRITICAL("bind error: {}", ErrnoStr());
    exit(EXIT_FAILURE);
  }
  SPDLOG_INFO("bind on '{}'", addr_);
}

void Acceptor::Listen() {
  // 接收连接
  int ret = listen(listen_fd_, kBackLog);
  if (ret == -1) {
    SPDLOG_CRITICAL("listen error: {}", ErrnoStr());
    exit(EXIT_FAILURE);
  }

  // 注册文件描述符
  loop_->AddFd(listen_fd_, std::bind(&Acceptor::OnAccept, shared_from_this()));
}

bool Acceptor::Accept(shared_ptr<Conn> conn) {
  sockaddr_in sock_addr;
  socklen_t sock_len = 0;
  int sock = accept4(listen_fd_, reinterpret_cast<struct sockaddr*>(&sock_addr),
                     &sock_len, SOCK_NONBLOCK);

  if (sock == -1) {
    if (errno != EAGAIN) {
      SPDLOG_CRITICAL("accept4 failed, listen_fd_={}, reason='{}'", listen_fd_,
                      ErrnoStr());
      exit(EXIT_FAILURE);
    } else {
      return false;
    }
  }

  Addr remote;
  bool ok = remote.FromSockAddrIn(sock_addr);
  if (!ok) {
    SPDLOG_CRITICAL("FromSockAddrIn error: {}", ErrnoStr());
    exit(EXIT_FAILURE);
  }

  conn->Connect(sock, std::move(remote));

  return true;
}

void Acceptor::AsyncAccept(shared_ptr<Conn> conn, AcceptHandler handler) {
  assert(!handler_);

  bool accepted = Accept(conn);
  if (accepted) {
    handler();
    return;
  }

  handler_ = std::move(handler);
  conn_ = conn;
}

void Acceptor::OnAccept(shared_ptr<Acceptor> acceptor) {
  SPDLOG_DEBUG("aa");
  // 如果没有异步接收连接的请求则直接返回
  if (!acceptor->handler_) {
    return;
  }

  auto handler = std::move(acceptor->handler_);
  bool accepted = acceptor->Accept(acceptor->conn_);
  if (accepted) {
    handler();
    return;
  }
  assert(false);
}

}  // namespace mydss::net
