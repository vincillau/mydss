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

#include <util/str.hpp>

using std::string;

namespace mydss::util {

size_t U64StrLen(uint64_t u64) {
  if (u64 == 0) {
    return 1;
  }

  size_t len = 0;
  while (u64 != 0) {
    u64 /= 10;
    len++;
  }
  return len;
}

bool StrToI64(const string& str, int64_t* result) {
  if (str.empty()) {
    return false;
  }

  int64_t i64 = 0;
  size_t index = 0;
  int sign = 1;

  if (str[0] == '-') {
    if (str.size() == 1) {
      return false;
    }
    index = 1;
    sign = -1;
  }

  for (; index < str.size(); index++) {
    char ch = str[index];
    if (ch < '0' || ch > '9') {
      return false;
    }

    // 检查是否会溢出
    if (i64 > INT64_MAX / 10) {
      return false;
    }
    i64 *= 10;

    int digit = ch - '0';
    // 检查是否会溢出
    if (INT64_MAX - i64 < digit) {
      return false;
    }
    i64 += digit;
  }

  *result = i64 * sign;
  return true;
}

}  // namespace mydss::util
