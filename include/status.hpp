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

#ifndef MYDSS_INCLUDE_STATUS_HPP_
#define MYDSS_INCLUDE_STATUS_HPP_

#include <fmt/core.h>
#include <fmt/ostream.h>

#include <string>

namespace mydss {

static constexpr int kOk = 0;
static constexpr int kBadReq = 1;

class Status {
 public:
  Status(int code, std::string msg) : code_(code), msg_(std::move(msg)) {}

  [[nodiscard]] auto code() const { return code_; }
  [[nodiscard]] const auto& msg() const { return msg_; }

  [[nodiscard]] bool ok() const { return code_ == kOk; }
  [[nodiscard]] bool error() const { return code_ != kOk; }

  static Status Ok() { return Status(0, "ok"); }

 private:
  int code_;
  std::string msg_;
};

inline static std::ostream& operator<<(std::ostream& os, const Status& status) {
  os << fmt::format("(status:{}) {}", status.code(), status.msg());
  return os;
}

}  // namespace mydss

#endif  // MYDSS_INCLUDE_STATUS_HPP_
