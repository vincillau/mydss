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

#include <functional>
#include <memory>
#include <unordered_map>

namespace mydss::net {

// 事件循环，监听文件描述符的读写事件，并在事件触发时调用 handler
// Loop 始终监听加入其中的读事件，而是否监听写事件由使用者决定
// 监听 fd 采用边缘触发模式
class Loop {
 public:
  using Handler = std::function<void()>;

  // 确保 Loop 对象一定被 std::shared_ptr 持有
  [[nodiscard]] static auto New() { return std::shared_ptr<Loop>(new Loop()); }

  // 将 fd 添加到事件循环，并注册读事件对应的 handler
  // 添加后事件循环将开始监听 fd 的读事件
  // handler 不能为空
  void Add(int fd, Handler handler);

  // 设置写事件的 handler
  // 如果 handler 不为空，开始监听写事件
  // 如果 handler 为空，停止监听写事件
  // 只能在 handler 为空时设置 handler
  // 只能在 handler 不为空时清空 handler
  void SetWriteHandler(int fd, Handler handler);

  // 从事件循环中移除 fd
  void Remove(int fd);

  // 运行事件循环
  [[noreturn]] void Run();

 private:
  // 创建 epoll fd
  Loop();

 private:
  // epoll 文件描述符
  int epfd_;
  // 添加到事件循环的文件描述符
  // key 为 fd，value 分别为该 fd 的读事件 handler 和写事件 handler
  std::unordered_map<int, std::pair<Handler, Handler>> fds_;
};

}  // namespace mydss::net

#endif  // MYDSS_INCLUDE_NET_LOOP_HPP_
