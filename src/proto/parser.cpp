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

#include <limit/limit.hpp>
#include <proto/parser.hpp>

using fmt::format;
using mydss::err::kBadReq;
using mydss::err::Status;
using mydss::limit::kMaxStrInReq;
using mydss::limit::kMaxStrLenInReq;
using std::isdigit;
using std::vector;

namespace mydss::proto {
void BulkStringParser::Reset() {
  state_ = State::kTypeChar;
  value_.clear();
  target_len_ = 0;
  cur_len_ = 0;
}

Status BulkStringParser::Step(char ch, bool* completed) {
  *completed = false;

  switch (state_) {
    case State::kTypeChar:
      if (ch == '$') {
        state_ = State::kLenFirstNum;
        return Status::Ok();
      }
      return Status(
          kBadReq,
          format("expect bulk string type char '$' instead of '{}'", ch));

    case State::kLenFirstNum:
      if (isdigit(ch)) {
        state_ = State::kLen;
        target_len_ = ch - '0';
        return Status::Ok();
      }
      return Status(kBadReq,
                    format("expect bulk string length instead of '{}'", ch));

    case State::kLen:
      if (isdigit(ch)) {
        target_len_ *= 10;
        target_len_ += ch - '0';
        if (target_len_ > kMaxStrLenInReq) {
          return Status(kBadReq, "bulk string is too long");
        }
        return Status::Ok();
      }
      if (ch == '\r') {
        state_ = State::kLenN;
        return Status::Ok();
      }
      return Status(
          kBadReq,
          format("expect '\r' after length of the bulk string instead of '{}'",
                 ch));

    case State::kLenN:
      if (ch == '\n') {
        state_ = State::kData;
        // 预分配内存
        value_.reserve(target_len_);
        return Status::Ok();
      }
      return Status(kBadReq, format("expect '\n' after bulk string "
                                    "length and '\r' instead of '{}'",
                                    ch));
    case State::kData:
      value_.push_back(ch);
      cur_len_++;
      if (cur_len_ == target_len_) {
        state_ = State::kR;
      }
      return Status::Ok();

    case State::kR:
      if (ch == '\r') {
        state_ = State::kN;
        return Status::Ok();
      }
      return Status(kBadReq, format("expect '\r' after bulk string "
                                    "value instead of '{}'",
                                    ch));
    case State::kN:
      if (ch == '\n') {
        *completed = true;
        return Status::Ok();
      }
      return Status(kBadReq, format("expect '\n' after bulk string "
                                    "value and '\r' instead of '{}'",
                                    ch));
  }
  return Status::Ok();
}

Status ReqParser::Parse(const char* buf, size_t len, vector<Req>& reqs) {
  for (size_t i = 0; i < len; i++) {
    bool completed = false;
    Status status = Step(buf[i], &completed);
    if (status.error()) {
      return status;
    }

    if (completed) {
      reqs.push_back(std::move(req_));
      Reset();
    }
  }
  return Status::Ok();
}

void ReqParser::Reset() {
  state_ = State::kArrayChar;
  req_ = Req();
  array_len_ = 0;
  str_parser_.Reset();
}

Status ReqParser::Step(char ch, bool* completed) {
  *completed = false;

  switch (state_) {
    case State::kArrayChar:
      if (ch == '*') {
        state_ = State::kArrayLenFirstNum;
        return Status::Ok();
      }
      return Status(kBadReq,
                    format("expect array type char '*' instead of '{}'", ch));

    case State::kArrayLenFirstNum:
      if (isdigit(ch)) {
        state_ = State::kArrayLen;
        array_len_ = ch - '0';
        return Status::Ok();
      }
      return Status(kBadReq, format("expect array length instead of '{}'", ch));

    case State::kArrayLen:
      if (isdigit(ch)) {
        array_len_ *= 10;
        array_len_ += ch - '0';
        if (array_len_ > kMaxStrInReq) {
          return Status(kBadReq, "array is too long");
        }
        return Status::Ok();
      }
      if (ch == '\r') {
        state_ = State::kArrayN;
        return Status::Ok();
      }
      return Status(
          kBadReq,
          format("expect '\r' after length of the array instead of '{}'", ch));

    case State::kArrayN:
      if (ch == '\n') {
        state_ = State::kStr;
        // 预分配内存
        req_.pieces().reserve(array_len_);
        return Status::Ok();
      }
      return Status(kBadReq, format("expect '\n' after array "
                                    "length and '\r' instead of '{}'",
                                    ch));

    case State::kStr:
      bool bs_completed = false;
      Status status = str_parser_.Step(ch, &bs_completed);
      if (status.error()) {
        return status;
      }

      if (!bs_completed) {
        return Status::Ok();
      }

      req_.pieces().push_back(str_parser_.MoveOut());
      str_parser_.Reset();
      if (req_.pieces().size() == array_len_) {
        *completed = true;
      }
      return Status::Ok();
  }
  return Status::Ok();
}

}  // namespace mydss::proto
