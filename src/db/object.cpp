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
#include <util.hpp>

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
