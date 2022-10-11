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

#ifndef MYDSS_INCLUDE_CONFIG_HPP_
#define MYDSS_INCLUDE_CONFIG_HPP_

#include <string>

namespace mydss {

class Config {
 public:
  [[nodiscard]] const auto& ip() const { return ip_; }
  void set_ip(std::string ip) { ip_ = std::move(ip); }

  [[nodiscard]] uint16_t port() const { return port_; }
  void set_port(uint16_t port) { port_ = port; }

  [[nodiscard]] int db_num() const { return db_num_; }
  void set_db_num(int db_num) { db_num_ = db_num; }

 private:
  std::string ip_ = "127.0.0.1";
  uint16_t port_ = 6379;
  int db_num_ = 16;
};

}  // namespace mydss

#endif  // MYDSS_INCLUDE_CONFIG_HPP_
