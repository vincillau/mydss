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

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <utility>

namespace mydss::net {

// 事件循环，负责监听文件描述符的事件并调用处理事件的回调
class Loop {
 public:
  using Callback = std::function<void(int /* MyDSS 错误码 */)>;

  // 初始化事件循环，创建 epoll fd
  Loop();

  // 异步等待 fd 的读事件，当事件发生时调用回调
  // 1. 如果没有注册事件，则注册读事件
  // 2. 如果已经注册读事件，则仅修改回调函数
  // 3. 如果仅注册写事件，则调用 epoll_ctl 修改监听的事件
  // hup_event 为 true 时监听 EPOLLRDHUP 事件
  void AwaitRead(int fd, Callback cb, bool hup_event);

  // 异步等待 fd 的写事件，当事件发生时调用回调
  // 1. 如果没有注册事件，则注册写事件
  // 2. 如果已经注册写事件，则仅修改回调函数
  // 3. 如果仅注册读事件，则调用 epoll_ctl 修改监听的事件
  // hup_event 为 true 时监听 EPOLLRDHUP 事件
  void AwaitWrite(int fd, Callback cb, bool hup_event);

  // 从事件循环中移除文件描述符
  void Remove(int fd);

  // 运行事件循环，在没有需要监听的文件描述符时返回
  void Run();

 private:
  // epoll 文件描述符
  int epfd_;
  // 被监听的文件描述符及其回调
  // key 为文件描述符，value 分别为读事件和写事件回调
  std::unordered_map<int, std::pair<Callback, Callback>> fd_cbs_map_;
};

}  // namespace mydss::net

#endif  // MYDSS_INCLUDE_NET_LOOP_HPP_
