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

#ifndef MYDSS_INCLUDE_UTIL_STR_HPP_
#define MYDSS_INCLUDE_UTIL_STR_HPP_

#include <string>

namespace mydss::util {

inline static void StrLower(std::string& str) {
  for (char& ch : str) {
    if (ch >= 'A' && ch <= 'Z') {
      ch = ch - 'A' + 'a';
    }
  }
};

// 计算 uint64_t 转换为字符串后的长度
[[nodiscard]] size_t U64StrLen(uint64_t u64);

// 计算 int64_t 转换为字符串后的长度
[[nodiscard]] inline size_t I64StrLen(int64_t i64) {
  if (i64 >= 0) {
    return U64StrLen(i64);
  }
  return 1 + U64StrLen(-i64);
}

[[nodiscard]] bool StrToI64(const std::string& str, int64_t* result);

}  // namespace mydss::util

#endif  // MYDSS_INCLUDE_UTIL_STR_HPP_
