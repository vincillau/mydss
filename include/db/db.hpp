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

#include <sys/time.h>

#include <list>
#include <string>
#include <unordered_map>
#include <vector>

#include "object.hpp"
#include "time_wheel.hpp"

namespace mydss {

constexpr std::size_t kTimeInterval = 5;
constexpr std::size_t kSlotNum = 12;

class Db {
 public:
  Db() : tw_(kTimeInterval, kSlotNum) {}

  [[nodiscard]] const auto& map() const { return map_; }
  [[nodiscard]] auto& map() { return map_; }

  void Timeout() {
    auto expired = tw_.Timeout();
    for (const auto& [key, _] : expired) {
      auto it = map_.find(key);
      if (it == map_.end()) {
        continue;
      }

      // ttl 可能已经被更改，需要重新检查
      if (!it->second.HasTtl() || it->second.pttl() <= 0) {
        continue;
      }

      map_.erase(key);
    }
  }

 private:
  std::unordered_map<std::string, Object> map_;
  TimeWheel tw_;
};

}  // namespace mydss

#endif  // MYDSS_INCLUDE_DB_DB_HPP_
