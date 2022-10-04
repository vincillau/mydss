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

#ifndef MYDSS_INCLUDE_UTIL_HPP_
#define MYDSS_INCLUDE_UTIL_HPP_

#include <string>

namespace mydss {

static void StrToUpper(std::string& str) {
  for (char& ch : str) {
    if (ch >= 'a' && ch <= 'z') {
      ch = ch - 'a' + 'A';
    }
  }
};

// 如果 str 表示一个 int64_t，返回 true；否则返回 false
static bool StrToI64(const std::string& str, int64_t* i64) {
  if (str.empty()) {
    return false;
  }

  *i64 = 0;
  int sign = 1;

  size_t i = 0;
  if (str[0] == '-') {
    if (str.size() == 1) {
      return false;
    }
    sign = -1;
  }

  for (; i < str.size(); i++) {
    if (!(str[i] >= '0' && str[i] <= '9')) {
      return false;
    }

    if (*i64 > INT64_MAX / 10) {
      return false;
    }
    *i64 *= 10;

    int digit = str[i] - '0';
    if (INT64_MAX - *i64 < digit) {
      return false;
    }
  }

  *i64 *= sign;
  return true;
}

}  // namespace mydss

#endif  // MYDSS_INCLUDE_UTIL_HPP_
