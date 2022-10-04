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

#include <spdlog/spdlog.h>
#include <sys/epoll.h>

#include <err/errno_str.hpp>
#include <net/loop.hpp>

using mydss::err::ErrnoStr;

namespace mydss::net {

Loop::Loop() : epfd_(epoll_create(1)) {
  if (epfd_ == -1) {
    SPDLOG_CRITICAL("epoll_create error: {}", ErrnoStr());
    exit(EXIT_FAILURE);
  }
}

void Loop::AwaitRead(int fd, Callback cb, bool et) {
  assert(cb);  // 回调不能为空

  epoll_event ev;
  ev.data.fd = fd;
  ev.events = EPOLLIN | (et ? EPOLLET : 0);
  auto fd_cbs = fd_cbs_map_.find(fd);

  // 1. 如果没有注册事件，则注册读事件，并设置回调函数
  if (fd_cbs == fd_cbs_map_.end()) {
    int ret = epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev);
    if (ret == -1) {
      SPDLOG_CRITICAL("epoll_ctl error: {}", ErrnoStr());
      exit(EXIT_FAILURE);
    }
  }
  // 2. 如果已经注册读事件，则仅修改回调函数
  else if (fd_cbs->second.first) {
  }
  // 3. 如果仅注册了写事件，则调用 epoll_ctl 修改监听的事件，并设置回调函数
  else {
    ev.events |= EPOLLOUT;
    int ret = epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev);
    if (ret == -1) {
      SPDLOG_CRITICAL("epoll_ctl error: {}", ErrnoStr());
      exit(EXIT_FAILURE);
    }
  }

  // 修改回调函数
  fd_cbs_map_[fd].first = std::move(cb);
}

void Loop::AwaitWrite(int fd, Callback cb) {
  assert(cb);  // 回调不能为空

  epoll_event ev;
  ev.data.fd = fd;
  ev.events = EPOLLOUT | EPOLLET;  // 使用边缘触发模式
  auto fd_cbs = fd_cbs_map_.find(fd);

  // 1. 如果没有注册事件，则注册写事件，并设置回调函数
  if (fd_cbs == fd_cbs_map_.end()) {
    int ret = epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev);
    if (ret == -1) {
      SPDLOG_CRITICAL("epoll_ctl error: {}", ErrnoStr());
      exit(EXIT_FAILURE);
    }
  }
  // 2. 如果已经注册写事件，则仅修改回调函数
  else if (fd_cbs->second.second) {
  }
  // 3. 如果仅注册了读事件，则调用 epoll_ctl 修改监听的事件，并设置回调函数
  else {
    ev.events |= EPOLLIN;
    int ret = epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev);
    if (ret == -1) {
      SPDLOG_CRITICAL("epoll_ctl error: {}", ErrnoStr());
      exit(EXIT_FAILURE);
    }
  }

  // 修改回调函数
  fd_cbs_map_[fd].second = std::move(cb);
}

void Loop::Remove(int fd) {
  int nerase = fd_cbs_map_.erase(fd);
  // fd 必须已经被事件循环监听
  assert(nerase == 1);
  int ret = epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr);
  if (ret == -1) {
    SPDLOG_CRITICAL("epoll_ctl error: {}", ErrnoStr());
    exit(EXIT_FAILURE);
  }
}

void Loop::Run() {
  for (;;) {
    // 没有被监听的文件描述符则直接退出循环
    if (fd_cbs_map_.empty()) {
      // 理论上 MyDSS 中的事件循环永远不会退出
      SPDLOG_ERROR("the event loop exited");
      break;
    }

    epoll_event ev;
    int ret = epoll_wait(epfd_, &ev, 1, -1);
    assert(ret != 0);  // epoll_wait 永不超时，所以不可能为 0
    if (ret == -1) {
      SPDLOG_CRITICAL("epoll_wait error: {}", ErrnoStr());
      exit(EXIT_FAILURE);
    }

    auto fd_cbs = fd_cbs_map_.find(ev.data.fd);
    assert(fd_cbs != fd_cbs_map_.end());
    const auto& read_cb = fd_cbs->second.first;
    const auto& write_cb = fd_cbs->second.second;

    // EPOLLERR 由回调处理
    if (ev.events & EPOLLIN) {
      // 必须有读事件回调
      assert(read_cb);
      read_cb();
    }
    if (ev.events & EPOLLOUT) {
      // 必须有写事件回调
      assert(write_cb);
      write_cb();
    }

    // 移除文件描述符
    Remove(ev.data.fd);
  }
}

}  // namespace mydss::net
