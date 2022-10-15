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

#ifndef MYDSS_INCLUDE_CMD_STRING_HPP_
#define MYDSS_INCLUDE_CMD_STRING_HPP_

#include <module/api.hpp>
#include <variant>

namespace mydss::cmd {

class String : public module::Object {
 public:
  String(std::string value = {})
      : Object(module::type::kString, module::encoding::kRaw) {
    SetValue(std::move(value));
  }

  [[nodiscard]] std::string TypeStr() const override { return "string"; }
  [[nodiscard]] std::string EncodingStr() const override;

  [[nodiscard]] int64_t I64() const {
    assert(encoding_ == module::encoding::kInt);
    return std::get<int64_t>(value_);
  }
  [[nodiscard]] const std::string& Str() const {
    assert(encoding_ == module::encoding::kRaw);
    return std::get<std::string>(value_);
  }

  void SetValue(std::string value);
  void SetI64(int64_t i64);

 public:
  static void Append(module::Ctx& ctx, module::Req req);
  static void Decr(module::Ctx& ctx, module::Req req);
  static void DecrBy(module::Ctx& ctx, module::Req req);
  static void Get(module::Ctx& ctx, module::Req req);
  static void GetDel(module::Ctx& ctx, module::Req req);
  static void GetRange(module::Ctx& ctx, module::Req req);
  static void Incr(module::Ctx& ctx, module::Req req);
  static void IncrBy(module::Ctx& ctx, module::Req req);
  static void MGet(module::Ctx& ctx, module::Req req);
  static void MSet(module::Ctx& ctx, module::Req req);
  static void MSetNx(module::Ctx& ctx, module::Req req);
  static void Set(module::Ctx& ctx, module::Req req);
  static void StrLen(module::Ctx& ctx, module::Req req);

 private:
  std::variant<std::string, int64_t> value_;
  uint16_t encoding_;
};

inline std::string String::EncodingStr() const {
  if (encoding_ == module::encoding::kInt) {
    return "int";
  }
  if (encoding_ == module::encoding::kRaw) {
    return "raw";
  }
  assert(false);
}

}  // namespace mydss::cmd

#endif  // MYDSS_INCLUDE_CMD_STRING_HPP_
