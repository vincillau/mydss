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

#ifndef MYDSS_INCLUDE_PROTO_REQ_HPP_
#define MYDSS_INCLUDE_PROTO_REQ_HPP_

#include <fmt/ostream.h>

#include <net/client.hpp>
#include <string>
#include <vector>

namespace mydss::proto {

// 请求对象，是一个 Bulk String 的数组
class Req {
 public:
  explicit Req(net::Client* client = nullptr) : client_(client) {}

  [[nodiscard]] const auto& pieces() const { return pieces_; }
  [[nodiscard]] auto& pieces() { return pieces_; }

  [[nodiscard]] const auto client() const { return client_; }
  void set_client(net::Client* client) { client_ = client; }

 private:
  std::vector<std::string> pieces_;
  net::Client* client_;
};

// 将 Req 的字符串表示形式写入 std::ostream，用于 fmt 库
inline static std::ostream& operator<<(std::ostream& os, const Req& req) {
  os << '(';
  for (size_t i = 0; i < req.pieces().size(); i++) {
    os << '\'' << req.pieces()[i] << '\'';
    if (i != req.pieces().size() - 1) {
      os << ", ";
    }
  }
  os << ')';
  return os;
}

}  // namespace mydss::proto

#endif  // MYDSS_INCLUDE_PROTO_REQ_HPP_
