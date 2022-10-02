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

#include <spdlog/spdlog.h>

#include <db/object.hpp>

using std::string;
using std::to_string;

namespace mydss {

string Object::str() const {
  assert(type_ == Type::kString);
  assert(encoding_ == Encoding::kInt || encoding_ == Encoding::kRaw);

  if (encoding_ == Encoding::kInt) {
    SPDLOG_DEBUG("access string object: encoding={}, value='{}'", encoding_,
                 std::get<Int>(data_));
    return to_string(std::get<Int>(data_));
  }
  SPDLOG_DEBUG("access string object: encoding={}, value='{}'", encoding_,
               std::get<Raw>(data_));
  return std::get<Raw>(data_);
}

namespace {

// 如果 str 表示一个 int64_t，返回 true；否则返回 false
static bool StrToI64(const std::string& str, int64_t* i64) {
  SPDLOG_DEBUG("str = '{}'", str);

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

}  // namespace

void Object::set_str(std::string str) {
  type_ = Type::kString;

  Int i64;
  bool is_num = StrToI64(str, &i64);
  if (is_num) {
    encoding_ = Encoding::kInt;
    data_ = i64;
    SPDLOG_DEBUG("set string object: encoding={}, value='{}'", encoding_,
                 std::get<Int>(data_));
  } else {
    encoding_ = Encoding::kRaw;
    data_ = std::move(str);
    SPDLOG_DEBUG("set string object: encoding={}, value='{}'", encoding_,
                 std::get<Raw>(data_));
  }
}

void Object::SetType(Type type) {
  type_ = type;
  switch (type) {
    case Type::kString:
      encoding_ = Encoding::kRaw;
      data_ = Raw{};
      break;
    case Type::kList:
      encoding_ = Encoding::kLinkedList;
      data_ = List{};
      break;
    case Type::kHash:
      encoding_ = Encoding::kHT;
      data_ = Hash{};
      break;
    case Type::kSet:
      encoding_ = Encoding::kHT;
      data_ = Set{};
      break;
    case Type::kZset:
      encoding_ = Encoding::kHT;
      data_ = Zset{};
      break;
  }
}

}  // namespace mydss
