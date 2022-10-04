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

#ifndef MYDSS_INCLUDE_NET_ADDR_HPP_
#define MYDSS_INCLUDE_NET_ADDR_HPP_

#include <arpa/inet.h>
#include <fmt/ostream.h>

#include <cstdint>
#include <string>

namespace mydss::net {

class Addr {
 public:
  Addr() : ip_{}, port_(0) {}
  Addr(std::string ip, uint16_t port) : ip_(std::move(ip)), port_(port) {}

  [[nodiscard]] const auto& ip() const { return ip_; }
  [[nodiscard]] auto port() const { return port_; }

  // 若成功返回 true，否则返回 false
  [[nodiscard]] bool ToSockAddrIn(sockaddr_in& sock_addr);
  // 若成功返回 true，否则返回 false
  [[nodiscard]] bool FromSockAddrIn(const sockaddr_in& sock_addr);

 private:
  std::string ip_;
  uint16_t port_;
};

// 向 std::ostream 输出 Addr 的字符串表示形式，用于 fmt 库
// 格式为 'ip:port'
inline static std::ostream& operator<<(std::ostream& os, const Addr& addr) {
  os << addr.ip() << ':' << addr.port();
  return os;
}

}  // namespace mydss::net

#endif  // MYDSS_INCLUDE_NET_ADDR_HPP_
