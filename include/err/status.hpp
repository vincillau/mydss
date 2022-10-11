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

#ifndef MYDSS_INCLUDE_ERR_STATUS_HPP_
#define MYDSS_INCLUDE_ERR_STATUS_HPP_

#include <fmt/core.h>

#include <string>

#include "code.hpp"

namespace mydss::err {

// 状态码
class Status {
 public:
  Status(int code, std::string msg) : code_(code), msg_(std::move(msg)) {}

  [[nodiscard]] auto code() const { return code_; }
  [[nodiscard]] const auto& msg() const { return msg_; }

  [[nodiscard]] bool ok() const { return code_ == kOk; }
  [[nodiscard]] bool error() const { return code_ != kOk; }

  // 返回 Status 的字符串表示形式
  [[nodiscard]] std::string ToString() {
    return fmt::format("(status:{}) {}", code_, msg_);
  }

  [[nodiscard]] static Status Ok() { return {kOk, "ok"}; }

 private:
  int code_;         // 错误码
  std::string msg_;  // 错误信息
};

}  // namespace mydss::err

#endif  // MYDSS_INCLUDE_ERR_STATUS_HPP_
