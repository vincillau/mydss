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

#ifndef MYDSS_INCLUDE_NET_CONN_HPP_
#define MYDSS_INCLUDE_NET_CONN_HPP_

#include "addr.hpp"
#include "loop.hpp"

namespace mydss::net {

class Conn {
 public:
  friend class Acceptor;

  // 保证所有的 Conn 对象都由 std::shared_ptr 持有
  [[nodiscard]] static auto New(std::shared_ptr<Loop> loop) {
    return std::shared_ptr<Conn>(new Conn(loop));
  }

 private:
  explicit Conn(std::shared_ptr<Loop> loop) : loop_(loop), sock_(-1) {}

 private:
  std::shared_ptr<Loop> loop_;
  int sock_;
  Addr remote_;
};

}  // namespace mydss::net

#endif  // MYDSS_INCLUDE_NET_CONN_HPP_
