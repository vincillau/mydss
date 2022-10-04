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
//

#ifndef MYDSS_INCLUDE_DB_TIME_WHEEL_HPP_
#define MYDSS_INCLUDE_DB_TIME_WHEEL_HPP_

#include <spdlog/spdlog.h>

#include <cassert>
#include <ctime>
#include <functional>
#include <list>
#include <string>
#include <vector>

namespace mydss {

class TimeWheel {
 public:
  using Item = typename std::pair<std::string, std::time_t>;

  TimeWheel(std::size_t interval, std::size_t slot_num)
      : slots_(slot_num), interval_(interval) {
    assert(interval);
  }

  void Add(std::string key, std::time_t expire);
  [[nodiscard]] std::list<Item> Timeout();

 private:
  std::vector<std::list<Item>> slots_;
  std::size_t interval_;
};

inline void TimeWheel::Add(std::string key, std::time_t expire) {
  std::size_t index = expire / interval_ / slots_.size();
  slots_[index].push_back({std::move(key), expire});
}

inline std::list<TimeWheel::Item> TimeWheel::Timeout() {
  std::list<Item> expired;
  std::time_t now = time(nullptr);
  std::size_t index = (now / interval_ - 1) % slots_.size();
  auto& slot = slots_[index];
  auto it = slot.cbegin();
  while (it != slot.cend()) {
    if (it->second <= now) {
      expired.push_back({std::move(it->first), it->second});
      it = slot.erase(it);
      continue;
    }
    it++;
  }

  return expired;
}

}  // namespace mydss

#endif  // MYDSS_INCLUDE_DB_TIME_WHEEL_HPP_
