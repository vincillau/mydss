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

#ifndef MYDSS_INCLUDE_MODULE_OBJECT_HPP_
#define MYDSS_INCLUDE_MODULE_OBJECT_HPP_

#include <cassert>
#include <string>

#include "time.hpp"

namespace mydss::module {

class Object {
 public:
  [[nodiscard]] uint16_t type() const { return type_; }
  [[nodiscard]] uint16_t encoding() const { return encoding_; }

  [[nodiscard]] virtual std::string TypeStr() const = 0;
  [[nodiscard]] virtual std::string EncodingStr() const = 0;

  [[nodiscard]] auto access_time() const { return access_time_; }
  [[nodiscard]] auto IdleTime() const { return TimeInMsec() - access_time_; }
  void Touch() { access_time_ = TimeInMsec(); }

  [[nodiscard]] auto expire_time() const { return expire_time_; }
  [[nodiscard]] int64_t PTtl() const;
  void SetPTtl(int64_t msec);

 protected:
  Object(uint16_t type, uint16_t encoding)
      : type_(type), encoding_(encoding), expire_time_(INT64_MAX) {
    Touch();
  }

 private:
  uint16_t type_;
  uint16_t encoding_;
  int64_t access_time_;
  int64_t expire_time_;
};

inline int64_t Object::PTtl() const {
  if (expire_time_ == INT64_MAX) {
    return -1;
  }
  int64_t now = TimeInMsec();
  if (expire_time_ < now) {
    return 0;
  }
  return expire_time_ - now;
}

inline void Object::SetPTtl(int64_t msec) {
  assert(msec >= -1);
  if (msec == -1) {
    expire_time_ = INT64_MAX;
    return;
  }
  expire_time_ = TimeInMsec() + msec;
}

}  // namespace mydss::module

#endif  // MYDSS_INCLUDE_MODULE_OBJECT_HPP_
