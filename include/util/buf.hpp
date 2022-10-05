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

#ifndef MYDSS_INCLUDE_UTIL_BUF_HPP_
#define MYDSS_INCLUDE_UTIL_BUF_HPP_

#include <cstddef>

namespace mydss::util {

class Buf {
 public:
  Buf() : data_(nullptr), len_(0) {}
  Buf(char* data, size_t len) : data_(data), len_(len) {}

  [[nodiscard]] const char* data() const { return data_; }
  [[nodiscard]] char* data() { return data_; }
  [[nodiscard]] auto len() const { return len_; }

 private:
  char* data_;
  size_t len_;
};

}  // namespace mydss::util

#endif  // MYDSS_INCLUDE_UTIL_BUF_HPP_
