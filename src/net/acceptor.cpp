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
#include <err/errno.hpp>
#include <net/acceptor.hpp>

using mydss::err::ErrnoStr;
using std::shared_ptr;

namespace mydss::net {

namespace {

static bool FdSetNonBlock(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags < 0) {
    return false;
  }
  flags |= O_NONBLOCK;
  return fcntl(fd, F_SETFL, flags) != -1;
}

}  // namespace

Acceptor::Acceptor(std::shared_ptr<Loop> loop)
    : loop_(loop), listen_fd_(socket(AF_INET, SOCK_STREAM, 0)) {
  // 创建监听套接字
  if (listen_fd_ == -1) {
    SPDLOG_CRITICAL("create socket error: {}", ErrnoStr());
    exit(EXIT_FAILURE);
  }

  // 设置非阻塞模式
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

void Acceptor::Bind(const Addr& addr) {
  // 转换地址
  sockaddr sock_addr;
  bool ok = addr.ToSockAddr(sock_addr);
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
  SPDLOG_INFO("bind on '{}'", addr);
}

void Acceptor::Listen(int backlog) {
  // 接收连接
  int ret = listen(listen_fd_, backlog);
  if (ret == -1) {
    SPDLOG_CRITICAL("listen error: {}", ErrnoStr());
    exit(EXIT_FAILURE);
  }

  // 注册文件描述符
  loop_->Add(listen_fd_, bind(&Acceptor::OnAccept, shared_from_this()));
}

bool Acceptor::Accept(shared_ptr<Conn> conn) {
  sockaddr sock_addr;
  socklen_t sock_len = 0;
  int sock = accept4(listen_fd_, &sock_addr, &sock_len, SOCK_NONBLOCK);
  if (sock == -1) {
    if (errno == EAGAIN) {
      return false;
    }
    SPDLOG_CRITICAL("accept4 failed, listen_fd_={}, reason='{}'", listen_fd_,
                    ErrnoStr());
    exit(EXIT_FAILURE);
  }

  Addr remote;
  bool ok = remote.FromSockAddr(sock_addr);
  if (!ok) {
    SPDLOG_CRITICAL("FromSockAddr error: {}", ErrnoStr());
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

  conn_ = conn;
  handler_ = std::move(handler);
}

void Acceptor::OnAccept(shared_ptr<Acceptor> acceptor) {
  assert(acceptor->handler_);

  auto handler = std::move(acceptor->handler_);
  bool accepted = acceptor->Accept(acceptor->conn_);
  assert(accepted);
  handler();
}

}  // namespace mydss::net
