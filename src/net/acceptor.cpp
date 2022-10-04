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
using mydss::util::FdSetNonblock;
using std::shared_ptr;

namespace mydss::net {

Acceptor::Acceptor(std::shared_ptr<Loop> loop, Addr addr)
    : loop_(loop),
      addr_(std::move(addr)),
      listen_fd_(socket(AF_INET, SOCK_STREAM, 0)) {
  if (listen_fd_ == -1) {
    SPDLOG_CRITICAL("create socket error: {}", ErrnoStr());
    exit(EXIT_FAILURE);
  }

  bool ok = FdSetNonblock(listen_fd_);
  if (!ok) {
    SPDLOG_CRITICAL("FdSetNonblock error: {}", ErrnoStr());
    exit(EXIT_FAILURE);
  }

  sockaddr_in sock_addr;
  ok = addr_.ToSockAddrIn(sock_addr);
  if (!ok) {
    SPDLOG_CRITICAL("inet_pton error: {}", ErrnoStr());
    exit(EXIT_FAILURE);
  }

  int ret =
      bind(listen_fd_, reinterpret_cast<const struct sockaddr*>(&sock_addr),
           sizeof(sock_addr));
  if (ret == -1) {
    SPDLOG_CRITICAL("bind error: {}", ErrnoStr());
    exit(EXIT_FAILURE);
  }
  SPDLOG_INFO("bind on '{}'", addr_);

  ret = listen(listen_fd_, kBackLog);
  if (ret == -1) {
    SPDLOG_CRITICAL("listen error: {}", ErrnoStr());
    exit(EXIT_FAILURE);
  }
}

void Acceptor::AsyncAccept(shared_ptr<Conn> conn, AcceptHandler handler) {
  loop_->AwaitRead(
      listen_fd_,
      std::bind(OnAccept, shared_from_this(), conn, std::move(handler)),
      false  // 接收连接事件使用水平触发
  );
}

void Acceptor::OnAccept(shared_ptr<Acceptor> acceptor, shared_ptr<Conn> conn,
                        AcceptHandler handler) {
  sockaddr_in sock_addr;
  socklen_t sock_len = 0;
  int sock = accept4(acceptor->listen_fd_,
                     reinterpret_cast<struct sockaddr*>(&sock_addr), &sock_len,
                     SOCK_NONBLOCK);
  if (sock == -1) {
    SPDLOG_CRITICAL("accept error: {}", ErrnoStr());
    exit(EXIT_FAILURE);
  }

  Addr remote;
  bool ok = remote.FromSockAddrIn(sock_addr);
  if (!ok) {
    SPDLOG_CRITICAL("FromSockAddrIn error: {}", ErrnoStr());
    exit(EXIT_FAILURE);
  }

  conn->sock_ = sock;
  conn->remote_ = std::move(remote);

  handler();
}

}  // namespace mydss::net
