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

#include <err/errno.hpp>
#include <net/loop.hpp>

using mydss::err::ErrnoStr;
using mydss::err::Status;

namespace mydss::net {

Status Loop::SetInEvent(int fd, Handler handler) {
  auto it = fds_.find(fd);
  if (it == fds_.end()) {
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLET | EPOLLIN;
    int ret = epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev);
    if (ret == -1) {
      return {errno, ErrnoStr()};
    }
    fds_[fd] = {std::move(handler), nullptr};
    return Status::Ok();
  }

  auto& handlers = it->second;
  uint64_t old_events = 0;
  uint64_t new_events = 0;
  if (handlers.first) {
    old_events |= EPOLLIN;
  }
  if (handlers.second) {
    old_events |= EPOLLOUT;
    new_events |= EPOLLOUT;
  }
  if (handler) {
    new_events |= EPOLLIN;
  }

  // 停止监听 fd
  if (new_events == 0) {
    int ret = epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr);
    if (ret == -1) {
      return {errno, ErrnoStr()};
    }
  }
  // 只修改 handler
  else if (old_events == new_events) {
    handlers.first = std::move(handler);
    return Status::Ok();
  }

  struct epoll_event ev;
  ev.data.fd = fd;
  ev.events = EPOLLET | new_events;
  int ret = epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev);
  if (ret == -1) {
    return {errno, ErrnoStr()};
  }
  handlers.first = std::move(handler);
  return Status::Ok();
}

Status Loop::SetOutEvent(int fd, Handler handler) {
  auto it = fds_.find(fd);
  if (it == fds_.end()) {
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLET | EPOLLOUT;
    int ret = epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev);
    if (ret == -1) {
      return {errno, ErrnoStr()};
    }
    fds_[fd] = {nullptr, std::move(handler)};
    return Status::Ok();
  }

  auto& handlers = it->second;
  uint64_t old_events = 0;
  uint64_t new_events = 0;
  if (handlers.first) {
    old_events |= EPOLLIN;
    new_events |= EPOLLIN;
  }
  if (handlers.second) {
    old_events |= EPOLLOUT;
  }
  if (handler) {
    new_events |= EPOLLOUT;
  }

  // 停止监听 fd
  if (new_events == 0) {
    int ret = epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr);
    if (ret == -1) {
      return {errno, ErrnoStr()};
    }
  }
  // 只修改 handler
  else if (old_events == new_events) {
    handlers.first = std::move(handler);
    return Status::Ok();
  }

  struct epoll_event ev;
  ev.data.fd = fd;
  ev.events = EPOLLET | new_events;
  int ret = epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev);
  if (ret == -1) {
    return {errno, ErrnoStr()};
  }
  handlers.second = std::move(handler);
  return Status::Ok();
}

Status Loop::Remove(int fd) {
  // 如果 fd 已被监听则移除
  if (fds_.find(fd) != fds_.end()) {
    int ret = epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr);
    if (ret == -1) {
      return {errno, ErrnoStr()};
    }
    fds_.erase(fd);
  }
  return Status::Ok();
}

void Loop::Run() {
  for (;;) {
    assert(fds_.size() > 0);

    epoll_event ev;
    int ret = epoll_wait(epfd_, &ev, 1, -1);
    assert(ret == 1);

    if (ev.events & EPOLLIN) {
      const auto& handler = fds_.at(ev.data.fd).first;
      assert(handler);
      handler();
    }

    if (ev.events & EPOLLOUT) {
      const auto& handler = fds_.at(ev.data.fd).second;
      assert(handler);
      handler();
    }
  }
}

}  // namespace mydss::net
