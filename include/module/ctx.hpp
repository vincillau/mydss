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

#ifndef MYDSS_INCLUDE_MODULE_CTX_HPP_
#define MYDSS_INCLUDE_MODULE_CTX_HPP_

#include <memory>

#include "object.hpp"
#include "piece.hpp"

namespace mydss::module {

class Ctx {
 public:
  explicit Ctx(uint64_t session_id) : session_id_(session_id) {}

  [[nodiscard]] std::shared_ptr<Object> GetObject(const std::string& key);
  void SetObject(const std::string& key, std::shared_ptr<Object> obj);
  bool DeleteObject(const std::string& key);
  void Reply(std::shared_ptr<Piece> piece);
  [[nodiscard]] int SelectDb(int db);
  void Close();
  const std::string& GetClientName();
  const void SetClientName(std::string name);
  const int64_t GetClientId();

 private:
  uint64_t session_id_;
};

}  // namespace mydss::module

#endif  // MYDSS_INCLUDE_MODULE_CTX_HPP_
