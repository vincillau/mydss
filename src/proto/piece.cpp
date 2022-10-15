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
#include <cstring>
#include <proto/piece.hpp>
#include <util/str.hpp>

using mydss::util::I64StrLen;
using mydss::util::U64StrLen;
using std::to_string;

namespace mydss::proto {

size_t SimpleStringPiece::Size() const {
  size_t result = 1;
  result += value_.size();
  result += 2;
  return result;
}

size_t SimpleStringPiece::Serialize(char* buf, size_t len) const {
  assert(len >= Size());

  buf[0] = '+';
  size_t offset = 1;
  memcpy(buf + 1, value_.c_str(), value_.size());
  offset += value_.size();
  buf[offset] = '\r';
  buf[offset + 1] = '\n';
  offset += 2;

  assert(offset == Size());
  return offset;
}

size_t ErrorPiece::Size() const {
  size_t result = 1;
  result += value_.size();
  result += 2;
  return result;
}

size_t ErrorPiece::Serialize(char* buf, size_t len) const {
  assert(len >= Size());

  buf[0] = '-';
  size_t offset = 1;
  memcpy(buf + 1, value_.c_str(), value_.size());
  offset += value_.size();
  buf[offset] = '\r';
  buf[offset + 1] = '\n';
  offset += 2;

  assert(offset == Size());
  return offset;
}

size_t IntegerPiece::Size() const {
  size_t result = 1;
  result += I64StrLen(value_);
  result += 2;
  return result;
}

size_t IntegerPiece::Serialize(char* buf, size_t len) const {
  assert(len >= Size());

  auto i64_str = to_string(value_);
  buf[0] = ':';
  size_t offset = 1;
  memcpy(buf + 1, i64_str.c_str(), i64_str.size());
  offset += i64_str.size();
  buf[offset] = '\r';
  buf[offset + 1] = '\n';
  offset += 2;

  assert(offset == Size());
  return offset;
}

size_t BulkStringPiece::Size() const {
  size_t result = 1;
  result += U64StrLen(value_.size());
  result += 2;
  result += value_.size();
  result += 2;
  return result;
}

size_t BulkStringPiece::Serialize(char* buf, size_t len) const {
  assert(len >= Size());

  buf[0] = '$';
  size_t offset = 1;
  auto len_str = to_string(value_.size());
  memcpy(buf + 1, len_str.c_str(), len_str.size());
  offset += len_str.size();
  buf[offset] = '\r';
  buf[offset + 1] = '\n';
  offset += 2;

  memcpy(buf + offset, value_.c_str(), value_.size());
  offset += value_.size();
  buf[offset] = '\r';
  buf[offset + 1] = '\n';
  offset += 2;

  assert(offset == Size());
  return offset;
}

size_t ArrayPiece::Size() const {
  size_t result = 1;
  result += U64StrLen(pieces_.size());
  result += 2;
  for (const auto& piece : pieces_) {
    result += piece->Size();
  }
  return result;
}

size_t ArrayPiece::Serialize(char* buf, size_t len) const {
  assert(len >= Size());

  buf[0] = '*';
  size_t offset = 1;
  auto len_str = to_string(pieces_.size());
  memcpy(buf + 1, len_str.c_str(), len_str.size());
  offset += len_str.size();
  buf[offset] = '\r';
  buf[offset + 1] = '\n';
  offset += 2;

  for (const auto& piece : pieces_) {
    offset += piece->Serialize(buf + offset, len - offset);
  }

  assert(offset == Size());
  return offset;
}

}  // namespace mydss::proto
