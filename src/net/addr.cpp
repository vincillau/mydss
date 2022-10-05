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

#include <cstring>
#include <net/addr.hpp>

namespace mydss::net {

bool Addr::ToSockAddr(sockaddr& sock_addr) const {
  memset(&sock_addr, 0, sizeof(sock_addr));
  auto sock_addr_in = reinterpret_cast<sockaddr_in*>(&sock_addr);
  sock_addr_in->sin_family = AF_INET;
  sock_addr_in->sin_port = htons(port_);  // 端口号需要是网络序
  return inet_pton(AF_INET, ip_.c_str(), &sock_addr_in->sin_addr) == 1;
}

bool Addr::FromSockAddr(const sockaddr& sock_addr) {
  char buf[kInetNtopBufSize] = {};
  const char* ret = inet_ntop(AF_INET, &sock_addr, buf, kInetNtopBufSize);
  if (ret == nullptr) {
    return false;
  }

  ip_ = buf;
  auto sock_addr_in = reinterpret_cast<const sockaddr_in*>(&sock_addr);
  port_ = ntohs(sock_addr_in->sin_port);  // 将网络序的端口号转换为主机序
  return true;
}

}  // namespace mydss::net
