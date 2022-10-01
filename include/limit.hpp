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

#ifndef MYDSS_INCLUDE_LIMIT_HPP_
#define MYDSS_INCLUDE_LIMIT_HPP_

#include <cstdint>

namespace mydss {

// 请求中字符串的最大数目
constexpr std::uint64_t kMaxStrInReq = UINT16_MAX;
// 请求中字符串的最大长度
constexpr std::uint64_t kStrLenMaxInReq = UINT16_MAX;

}  // namespace mydss

#endif  // MYDSS_INCLUDE_LIMIT_HPP_
