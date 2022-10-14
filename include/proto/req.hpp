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

#include <cassert>
#include <server/client.hpp>
#include <string>
#include <vector>

namespace mydss::proto {

// 请求对象，是一个 Bulk String 的数组
class Req {
 public:
  explicit Req(server::Client* client = nullptr) : client_(client) {}

  [[nodiscard]] const auto& pieces() const { return pieces_; }
  [[nodiscard]] auto& pieces() { return pieces_; }

  [[nodiscard]] auto client() const { return client_; }
  void set_client(std::shared_ptr<server::Client> client) { client_ = client; }

  [[nodiscard]] std::string ToString() const;

 private:
  std::vector<std::string> pieces_;
  std::shared_ptr<server::Client> client_;
};

inline std::string Req::ToString() const {
  assert(pieces_.size() > 0);

  std::string str = "('" + pieces_.front() + "'";
  for (size_t i = 1; i < pieces_.size(); i++) {
    str += ", '" + pieces_[i] + "'";
  }
  str.push_back(')');
  return str;
}

}  // namespace mydss::proto

#endif  // MYDSS_INCLUDE_PROTO_REQ_HPP_
