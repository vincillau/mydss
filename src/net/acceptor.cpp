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

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <err/errno.hpp>
#include <net/acceptor.hpp>

using fmt::format;
using mydss::err::ErrnoStr;
using mydss::err::kInvalidAddr;
using mydss::err::Status;
using std::shared_ptr;

namespace mydss::net {

static Status CreateSocket(InetType type, int& fd) {
  auto domain = type == InetType::kIPv4 ? AF_INET : AF_INET6;
  fd = socket(domain, SOCK_STREAM, 0);
  if (fd == -1) {
    return {errno, ErrnoStr()};
  }
  return Status::Ok();
}

// 设置非阻塞模式
static Status SetNonBlock(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags < -1) {
    return {errno, ErrnoStr()};
  }
  flags |= O_NONBLOCK;
  int ret = fcntl(fd, F_SETFL, flags);
  if (ret == -1) {
    return {errno, ErrnoStr()};
  }
  return Status::Ok();
}

// 设置 SO_REUSEADDR 标志
static Status SetReuseAddr(int fd) {
  int opt = 1;
  int ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  if (ret == -1) {
    return {errno, ErrnoStr()};
  }
  return Status::Ok();
}

static Status BindIPv4(int fd, const EndPoint& ep) {
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(ep.port());
  int ret = inet_pton(AF_INET, ep.ip().c_str(), &addr.sin_addr);
  if (ret == 0) {
    return {kInvalidAddr, format("invalid IPv4 address '{}'", ep.ip())};
  }

  ret = bind(fd, reinterpret_cast<const struct sockaddr*>(&addr), sizeof(addr));
  if (ret == -1) {
    return {errno, ErrnoStr()};
  }
  return Status::Ok();
}

static Status BindIPv6(int fd, const EndPoint& ep) {
  struct sockaddr_in6 addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin6_family = AF_INET6;
  addr.sin6_port = htons(ep.port());
  int ret = inet_pton(AF_INET6, ep.ip().c_str(), &addr.sin6_addr);
  if (ret == 0) {
    return {kInvalidAddr, format("invalid IPv6 address '{}'", ep.ip())};
  }

  ret = bind(fd, reinterpret_cast<const struct sockaddr*>(&addr), sizeof(addr));
  if (ret == -1) {
    return {errno, ErrnoStr()};
  }
  return Status::Ok();
}

// 将套接字绑定到 EndPoint
static Status Bind(int fd, const EndPoint& ep) {
  if (ep.type() == InetType::kIPv4) {
    return BindIPv4(fd, ep);
  }
  return BindIPv6(fd, ep);
}

static Status Listen(int fd, int backlog) {
  int ret = listen(fd, backlog);
  if (ret == -1) {
    return {errno, ErrnoStr()};
  }
  return Status::Ok();
}

Status Acceptor::Start(int backlog) {
  auto status = CreateSocket(ep_.type(), listen_fd_);
  if (status.error()) {
    return status;
  }

  status = SetNonBlock(listen_fd_);
  if (status.error()) {
    return status;
  }

  status = SetReuseAddr(listen_fd_);
  if (status.error()) {
    return status;
  }

  status = Bind(listen_fd_, ep_);
  if (status.error()) {
    return status;
  }

  status = Listen(listen_fd_, backlog);
  if (status.error()) {
    return status;
  }

  return Status::Ok();
}

void Acceptor::AsyncAccept(shared_ptr<Conn> conn, AcceptHandler handler) {
  assert(handler);

  if (reqs_.size() > 0) {
    reqs_.emplace_back(conn, std::move(handler));
    return;
  }

  auto status = Accept(conn);
  if (status.ok() || status.code() != EAGAIN) {
    handler(std::move(status));
    return;
  }

  reqs_.emplace_back(conn, std::move(handler));
  status = loop_->SetInEvent(listen_fd_, bind(OnAccept, shared_from_this()));
  if (status.error()) {
    handler(std::move(status));
  }
}

Status Acceptor::Accept(shared_ptr<Conn> conn) {
  EndPoint remote;
  int sock = -1;

  if (ep_.type() == InetType::kIPv4) {
    sockaddr_in addr;
    socklen_t len = 0;
    sock = accept4(listen_fd_, reinterpret_cast<struct sockaddr*>(&addr), &len,
                   SOCK_NONBLOCK);
    if (sock == -1) {
      return {errno, ErrnoStr()};
    }

    char buf[20] = {};
    auto ret = inet_ntop(AF_INET, &addr, buf, 20);
    if (ret == nullptr) {
      return {errno, ErrnoStr()};
    }
    remote.set_type(InetType::kIPv4);
    remote.set_ip(buf);
    remote.set_port(ntohs(addr.sin_port));
  } else {
    sockaddr_in6 addr;
    socklen_t len = 0;
    sock = accept4(listen_fd_, reinterpret_cast<struct sockaddr*>(&addr), &len,
                   SOCK_NONBLOCK);
    if (sock == -1) {
      return {errno, ErrnoStr()};
    }

    char buf[40] = {};
    auto ret = inet_ntop(AF_INET6, &addr, buf, 40);
    if (ret == nullptr) {
      return {errno, ErrnoStr()};
    }
    remote.set_type(InetType::kIPv6);
    remote.set_ip(buf);
    remote.set_port(ntohs(addr.sin6_port));
  }

  conn->Connect(sock, std::move(remote));
  conn->Attach(loop_);
  return Status::Ok();
}

void Acceptor::OnAccept(shared_ptr<Acceptor> acceptor) {
  assert(acceptor->reqs_.size() > 0);

  while (acceptor->reqs_.size() > 0) {
    auto& req = acceptor->reqs_.front();
    auto status = acceptor->Accept(req.conn());
    if (status.code() == EAGAIN) {
      return;
    }
    req.handler()(status);
    acceptor->reqs_.pop_front();
  }

  // 没有待处理的建立连接请求，停止等待可读事件
  auto status = acceptor->loop_->SetInEvent(acceptor->listen_fd_, nullptr);
  assert(status.ok());
}

}  // namespace mydss::net
