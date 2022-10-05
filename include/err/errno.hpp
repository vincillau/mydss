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

#include <cerrno>
#include <cstddef>
#include <cstring>
#include <string>

#ifndef MYDSS_INCLUDE_ERR_ERRNO_HPP_
#define MYDSS_INCLUDE_ERR_ERRNO_HPP_

namespace mydss::err {

constexpr size_t kErrnoStrBufSize = 64;

// 线程安全版本的 str_error
inline static std::string ErrnoStr() {
  char buf[kErrnoStrBufSize] = {};
  const char* err_str = strerror_r(errno, buf, kErrnoStrBufSize);
  return {err_str};
}

}  // namespace mydss::err

#endif  // MYDSS_INCLUDE_ERR_ERRNO_HPP_
