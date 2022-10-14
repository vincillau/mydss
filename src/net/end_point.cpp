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

#include <cassert>
#include <cstring>
#include <err/code.hpp>
#include <net/end_point.hpp>

using fmt::format;
using mydss::err::kInvalidAddr;
using mydss::err::Status;
using std::string;

namespace mydss::net {

Status EndPoint::ToSockAddr(struct sockaddr& sockaddr) const {
  memset(&sockaddr, 0, sizeof(sockaddr));

  if (type_ == InetType::kIPv4) {
    auto sockaddr_in = reinterpret_cast<struct sockaddr_in*>(&sockaddr);
    sockaddr_in->sin_family = AF_INET;
    // 端口号是网络序
    sockaddr_in->sin_port = htons(port_);

    // inet_pton 成功返回 1，无效的 IP 地址返回 0，af 参数不正确返回 -1
    int ret = inet_pton(AF_INET, ip_.c_str(), &sockaddr_in->sin_addr);
    if (ret == 1) {
      return Status::Ok();
    } else if (ret == 0) {
      return {kInvalidAddr, format("invalid IPv4 address '{}'", ip_)};
    } else {
      assert(false);
    }
  } else if (type_ == InetType::kIPv6) {
    auto sockaddr_in6 = reinterpret_cast<struct sockaddr_in6*>(&sockaddr);
    sockaddr_in6->sin6_family = AF_INET6;
    // 端口号是网络序
    sockaddr_in6->sin6_port = htons(port_);

    // inet_pton 成功返回 1，无效的 IP 地址返回 0，af 参数不正确返回 -1
    int ret = inet_pton(AF_INET6, ip_.c_str(), &sockaddr_in6->sin6_addr);
    if (ret == 1) {
      return Status::Ok();
    } else if (ret == 0) {
      return {kInvalidAddr, format("invalid IPv6 address '{}'", ip_)};
    } else {
      assert(false);
    }
  }

  assert(false);
}

static constexpr size_t kInetNtopBufSize = 64;

Status EndPoint::FromSockAddr(const struct sockaddr& sockaddr, InetType type) {
  char buf[kInetNtopBufSize] = {};

  if (type == InetType::kIPv4) {
    const char* ret = inet_ntop(AF_INET, &sockaddr, buf, kInetNtopBufSize);
    if (ret == nullptr) {
      return {kInvalidAddr, format("invalid IPv4 address")};
    }
    const auto* sockaddr_in =
        reinterpret_cast<const struct sockaddr_in*>(&sockaddr);
    // 将网络序的端口号转换为主机序
    port_ = ntohs(sockaddr_in->sin_port);
  } else if (type == InetType::kIPv6) {
    const char* ret = inet_ntop(AF_INET6, &sockaddr, buf, kInetNtopBufSize);
    if (ret == nullptr) {
      return {kInvalidAddr, format("invalid IPv6 address")};
    }
    const auto* sockaddr_in6 =
        reinterpret_cast<const struct sockaddr_in6*>(&sockaddr);
    // 将网络序的端口号转换为主机序
    port_ = ntohs(sockaddr_in6->sin6_port);
  }

  type_ = type;
  ip_ = string(buf);
  return Status::Ok();
}

}  // namespace mydss::net
