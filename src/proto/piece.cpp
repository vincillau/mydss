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

#include <cassert>
#include <proto/piece.hpp>

using std::to_string;

namespace mydss::proto {

size_t SimpleStringPiece::size() const {
  size_t result = 1;        // 表示类型的字符
  result += value_.size();  // 字符串
  result += 2;              // \r\n
  return result;
}

void SimpleStringPiece::Serialize(std::string& buf) const {
  buf.push_back('+');
  buf += value_;
  buf += "\r\n";
}

size_t ErrorPiece::size() const {
  size_t result = 1;        // 表示类型的字符
  result += value_.size();  // 字符串
  result += 2;              // \r\n
  return result;
}

void ErrorPiece::Serialize(std::string& buf) const {
  buf.push_back('-');
  buf += value_;
  buf += "\r\n";
}

size_t IntegerPiece::size() const {
  size_t result = 1;                   // 表示类型的字符
  result += to_string(value_).size();  // 转换为字符串后的长度
  result += 2;                         // \r\n
  return result;
}

void IntegerPiece::Serialize(std::string& buf) const {
  buf.push_back(':');
  buf += to_string(value_);
  buf += "\r\n";
}

size_t BulkStringPiece::size() const {
  if (null_) {
    // "$-1\r\n"
    return 5;
  }

  size_t result = 1;                          // 表示类型的字符
  result += to_string(value_.size()).size();  // 长度
  result += 2;                                // \r\n
  result += value_.size();                    // 字符串
  result += 2;                                // \r\n
  return result;
}

void BulkStringPiece::Serialize(std::string& buf) const {
  buf.push_back('$');
  buf += to_string(value_.size());
  buf += "\r\n";
  buf += value_;
  buf += "\r\n";
}

size_t ArrayPiece::size() const {
  size_t result = 1;                           // 表示类型的字符
  result += to_string(pieces_.size()).size();  // 长度
  result += 2;                                 // \r\n
  for (const auto& piece : pieces_) {
    result += piece->size();
  }
  return result;
}

void ArrayPiece::Serialize(std::string& buf) const {
  buf.reserve(size());
  buf.push_back('*');
  buf += to_string(pieces_.size());
  buf += "\r\n";
  for (const auto& piece : pieces_) {
    piece->Serialize(buf);
  }
}

}  // namespace mydss::proto
