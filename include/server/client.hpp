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

#ifndef MYDSS_INCLUDE_SERVER_CLIENT_HPP_
#define MYDSS_INCLUDE_SERVER_CLIENT_HPP_

#include <cstdint>
#include <string>

namespace mydss::server {

// 表示一个客户端，存储与此客户端相关的信息
class Client {
 public:
  Client() : id_(next_id_) { next_id_++; }

  [[nodiscard]] int64_t id() const { return id_; }
  [[nodiscard]] const auto& name() const { return name_; }
  void set_name(std::string name) { name_ = std::move(name); }

 private:
  // 下一个客户端的连接 ID
  static int64_t next_id_;

 private:
  int64_t id_;        // 客户端的连接 ID
  std::string name_;  // 客户端的名称
};

}  // namespace mydss::server

#endif  // MYDSS_INCLUDE_SERVER_CLIENT_HPP_
