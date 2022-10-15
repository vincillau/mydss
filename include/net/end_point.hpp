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

#ifndef MYDSS_INCLUDE_NET_END_POINT_HPP_
#define MYDSS_INCLUDE_NET_END_POINT_HPP_

#include <arpa/inet.h>
#include <fmt/core.h>

#include <cstdint>
#include <err/status.hpp>
#include <string>

#include "inet.hpp"

namespace mydss::net {

// 通信端点，包括 IP 地址和端口号
// 支持 IPv4 和 IPv6
class EndPoint {
 public:
  EndPoint() : type_(InetType::kIPv4), ip_{}, port_(0) {}
  EndPoint(InetType type, std::string ip, uint16_t port)
      : type_(type), ip_(std::move(ip)), port_(port) {}

  [[nodiscard]] auto type() const { return type_; }
  [[nodiscard]] const auto& ip() const { return ip_; }
  [[nodiscard]] auto port() const { return port_; }

  // 获取 EndPoint 的字符串表示形式
  [[nodiscard]] std::string ToString() const {
    return fmt::format("({}, {})", ip_, port_);
  }

  // 将通信端点的表示形式从 EndPoint 转换为 struct sockaddr
  // 可从返回值判断是否转换成功以及出错原因
  [[nodiscard]] err::Status ToSockAddr(struct sockaddr& sockaddr) const;

  // 将通信端点的表示形式从 struct sockaddr 转换为 EndPoint
  // 可从返回值判断是否转换成功以及出错原因
  [[nodiscard]] err::Status FromSockAddr(const struct sockaddr& sockaddr,
                                         InetType type);

 private:
  InetType type_;
  std::string ip_;  // 点分 IPv4 地址
  uint16_t port_;   // 端口号
};

}  // namespace mydss::net

#endif  // MYDSS_INCLUDE_NET_END_POINT_HPP_
