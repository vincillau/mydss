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

#ifndef MYDSS_INCLUDE_NET_LOOP_HPP_
#define MYDSS_INCLUDE_NET_LOOP_HPP_

#include <sys/epoll.h>

#include <cassert>
#include <err/status.hpp>
#include <functional>
#include <memory>
#include <unordered_map>

namespace mydss::net {

// 事件循环，监听文件描述符的读写事件，并在事件触发时调用对应的 handler
// 监听采用边缘触发模式
class Loop {
 public:
  using Handler = std::function<void()>;

  // 确保 Loop 对象一定被 std::shared_ptr 持有
  [[nodiscard]] static auto New() { return std::shared_ptr<Loop>(new Loop()); }

  // 设置读事件的 handler
  [[nodiscard]] err::Status SetInEvent(int fd, Handler handler);
  // 设置写事件的 handler
  [[nodiscard]] err::Status SetOutEvent(int fd, Handler handler);
  // 从事件循环中移除 fd
  [[nodiscard]] err::Status Remove(int fd);
  // 判断 fd 是否被 Loop 监听
  [[nodiscard]] bool Contains(int fd) { return fds_.find(fd) != fds_.end(); }

  // 运行事件循环
  [[noreturn]] void Run();

 private:
  Loop() : epfd_(epoll_create(1)) { assert(epfd_ != -1); }

 private:
  // epoll 文件描述符
  int epfd_;
  // 添加到事件循环的文件描述符
  // key 为 fd，value 分别为该 fd 的读事件 handler 和写事件 handler
  std::unordered_map<int, std::pair<Handler, Handler>> fds_;
};

}  // namespace mydss::net

#endif  // MYDSS_INCLUDE_NET_LOOP_HPP_
