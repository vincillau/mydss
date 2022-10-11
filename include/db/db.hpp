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

#ifndef MYDSS_INCLUDE_DB_DB_HPP_
#define MYDSS_INCLUDE_DB_DB_HPP_

#include <memory>
#include <string>
#include <unordered_map>

#include "object.hpp"

namespace mydss::db {

class Db {
 public:
  [[nodiscard]] const auto& objs() const { return objs_; }
  [[nodiscard]] auto& objs() { return objs_; }

 private:
  std::unordered_map<std::string, std::shared_ptr<Object>> objs_;
};

}  // namespace mydss::db

#endif  // MYDSS_INCLUDE_DB_DB_HPP_
