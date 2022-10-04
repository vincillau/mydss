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
#include <stdlib.h>
#include <sys/epoll.h>

#include <cassert>
#include <cerrno>
#include <cstring>
#include <err/err_code.hpp>
#include <err/errno_str.hpp>
#include <net/loop.hpp>

using mydss::err::ErrnoStr;
using mydss::err::kEof;
using mydss::err::kOk;

namespace mydss::net {

Loop::Loop() : epfd_(epoll_create(1)) {
  // 如果 epoll_create 出错则直接退出程序
  if (epfd_ == -1) {
    SPDLOG_CRITICAL("epoll_create error: {}", ErrnoStr());
    exit(EXIT_FAILURE);
  }
}

void Loop::AwaitRead(int fd, Callback cb, bool hup_event) {
  assert(cb);  // 回调不能为空

  auto fd_cbs = fd_cbs_map_.find(fd);
  epoll_event ev;
  ev.events = hup_event ? EPOLLRDHUP : 0;
  ev.events |= EPOLLIN | EPOLLET;  // 使用边缘触发模式

  // 1. 如果没有注册事件，则注册读事件
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
  // 3. 如果仅注册写事件，则调用 epoll_ctl 修改监听的事件
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

void Loop::AwaitWrite(int fd, Callback cb, bool hup_event) {
  assert(cb);  // 回调不能为空

  auto fd_cbs = fd_cbs_map_.find(fd);
  epoll_event ev;
  ev.events = hup_event ? EPOLLRDHUP : 0;
  ev.events |= EPOLLOUT | EPOLLET;  // 使用边缘触发模式

  // 1. 如果没有注册事件，则注册写事件
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
  // 3. 如果仅注册读事件，则调用 epoll_ctl 修改监听的事件
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
  assert(nerase == 1);
  int ret = epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr);
  if (ret == -1) {
    SPDLOG_CRITICAL("epoll_ctl error: {}", ErrnoStr());
    exit(EXIT_FAILURE);
  }
}

void Loop::Run() {
  for (;;) {
    // 没有被监听的文件描述符则直接退出
    if (fd_cbs_map_.empty()) {
      break;
    }

    epoll_event ev;
  retry:
    int ret = epoll_wait(epfd_, &ev, 1, -1);
    assert(ret != 0);  // epoll_wait 永不超时，所以不可能为 0
    if (ret == -1) {
      // 如果被信号中断，则重试
      if (errno == EINTR) {
        goto retry;
      }
      SPDLOG_CRITICAL("epoll_wait error: {}", ErrnoStr());
      exit(EXIT_FAILURE);
    }

    auto fd_cbs = fd_cbs_map_.find(ev.data.fd);
    assert(fd_cbs != fd_cbs_map_.end());

    // 在 IO 线程可能需要发送的数据，并且此时也可以接收数据
    // 所以允许同时监听读事件和写事件
    // TODO (Vincil Lau): 处理 EPOLLERR 和 EPOLLHUB 事件
    if (ev.events & EPOLLIN) {
      const Callback& read_cb = fd_cbs->second.first;
      assert(read_cb);  // 必须有读事件回调
      int err_code = kOk;
      if (ev.events | EPOLLERR) {
        if (ev.events | EPOLLHUP) {
          err_code = kEof;
        } else {
          SPDLOG_CRITICAL("listen EPOLLIN event error: {}", ErrnoStr());
          exit(EXIT_FAILURE);
        }
      }
      read_cb(err_code);
    }

    if (ev.events & EPOLLOUT) {
      const Callback& write_cb = fd_cbs->second.second;
      assert(write_cb);  // 必须有写事件回调
      if (ev.events | EPOLLERR) {
        SPDLOG_CRITICAL("listen EPOLLIN event error: {}", ErrnoStr());
        exit(EXIT_FAILURE);
      }
      write_cb(kOk);
    }

    // 移除文件描述符
    ret = epoll_ctl(epfd_, EPOLL_CTL_DEL, ev.data.fd, nullptr);
    if (ret == -1) {
      SPDLOG_CRITICAL("epoll_ctl error: {}", ErrnoStr());
      exit(EXIT_FAILURE);
    }
    fd_cbs_map_.erase(fd_cbs);
  }
}

}  // namespace mydss::net
