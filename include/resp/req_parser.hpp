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

#ifndef MYDSS_INCLUDE_RESP_REQ_PARSER_HPP_
#define MYDSS_INCLUDE_RESP_REQ_PARSER_HPP_

#include <spdlog/spdlog.h>

#include <cctype>
#include <limit.hpp>
#include <status.hpp>
#include <vector>

#include "req.hpp"

namespace mydss {

class BulkStringParser {
 public:
  void Reset();
  Status Step(char ch, bool* completed);
  std::string MoveOut() { return std::move(value_); }

 private:
  enum class State {
    kTypeChar,     // 期待接收表示 bluk string 类型的字符，即 '$'
    kLenFirstNum,  // 期待接收到字符串长度的第一个数字字符
    kLen,          // 正在接收字符串长度
    kLenN,         // 期待接收字符串长度后的 '\n'
    kData,         // 接收字符串的数据部分
    kR,            // 期待接收到字符串结尾的 '\r'
    kN             // 期待接收到字符串结尾的 '\n'
  };

 private:
  State state_;
  std::string value_;
  std::uint64_t target_len_;
  std::uint64_t cur_len_;
};

inline void BulkStringParser::Reset() {
  state_ = State::kTypeChar;
  value_.clear();
  target_len_ = 0;
  cur_len_ = 0;
}

inline Status BulkStringParser::Step(char ch, bool* completed) {
  *completed = false;

  switch (state_) {
    case State::kTypeChar:
      if (ch == '$') {
        state_ = State::kLenFirstNum;
        return Status::Ok();
      }
      return Status(
          kBadReq,
          fmt::format("expect bulk string type char '$' instead of '{}'", ch));
    case State::kLenFirstNum:
      if (std::isdigit(ch)) {
        state_ = State::kLen;
        target_len_ = ch - '0';
        return Status::Ok();
      }
      return Status(
          kBadReq,
          fmt::format("expect bulk string length instead of '{}'", ch));
    case State::kLen:
      if (std::isdigit(ch)) {
        if (target_len_ > kStrLenMaxInReq / 10) {
          return Status(kBadReq, "bulk string is too long");
        }
        target_len_ *= 10;
        target_len_ += ch - '0';
        return Status::Ok();
      }
      if (ch == '\r') {
        state_ = State::kLenN;
        return Status::Ok();
      }
      return Status(
          kBadReq,
          fmt::format(
              "expect '\r' after length of the bulk string instead of '{}'",
              ch));
    case State::kLenN:
      if (ch == '\n') {
        state_ = State::kData;
        value_.reserve(target_len_);
        return Status::Ok();
      }
      return Status(kBadReq, fmt::format("expect '\n' after bulk string "
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
      return Status(kBadReq, fmt::format("expect '\r' after bulk string "
                                         "value instead of '{}'",
                                         ch));
    case State::kN:
      if (ch == '\n') {
        *completed = true;
        return Status::Ok();
      }
      return Status(kBadReq, fmt::format("expect '\n' after bulk string "
                                         "value and '\r' instead of '{}'",
                                         ch));
  }
  return Status::Ok();
}

class ReqParser {
 public:
  ReqParser() { Reset(); }
  Status Parse(const char* buf, std::size_t len, std::vector<Req>& reqs);

 private:
  enum class State {
    kArrayChar,  // 期待下一个接收的字符为表示数组的字符，即 '*'
    kArrayLenFirstNum,  // 期待接收到数组长度的第一个数字字符
    kArrayLen,          // 正在接收数组长度
    kArrayN,            // 期待接收数组长度后的 \n
    kStr                // 接收字符串
  };

  void Reset();
  Status Step(char ch, bool* completed);

 private:
  State state_;
  BulkStringParser str_parser_;
  std::vector<std::string> strs_;
  std::uint64_t array_len_;
  std::uint64_t str_count_;
};

inline Status ReqParser::Parse(const char* buf, std::size_t len,
                               std::vector<Req>& reqs) {
  SPDLOG_DEBUG("parse '{}', len={}", std::string(buf, len), len);

  for (size_t i = 0; i < len; i++) {
    bool completed = false;
    Status status = Step(buf[i], &completed);
    if (status.error()) {
      return status;
    }
    if (completed) {
      Req req;
      for (auto& str : strs_) {
        req.pieces().emplace_back(std::move(str));
      }
      reqs.emplace_back(std::move(req));
      Reset();
    }
  }
  return Status::Ok();
}

inline void ReqParser::Reset() {
  state_ = State::kArrayChar;
  str_parser_.Reset();
  strs_.clear();
  array_len_ = 0;
  str_count_ = 0;
}

inline Status ReqParser::Step(char ch, bool* completed) {
  *completed = false;

  switch (state_) {
    case State::kArrayChar:
      if (ch == '*') {
        state_ = State::kArrayLenFirstNum;
        return Status::Ok();
      }
      return Status(
          kBadReq,
          fmt::format("expect array type char '*' instead of '{}'", ch));
    case State::kArrayLenFirstNum:
      if (std::isdigit(ch)) {
        state_ = State::kArrayLen;
        array_len_ = ch - '0';
        return Status::Ok();
      }
      return Status(kBadReq,
                    fmt::format("expect array length instead of '{}'", ch));
    case State::kArrayLen:
      if (std::isdigit(ch)) {
        if (array_len_ > kMaxStrInReq / 10) {
          return Status(kBadReq, "array is too long");
        }
        array_len_ *= 10;
        array_len_ += ch - '0';
        return Status::Ok();
      }
      if (ch == '\r') {
        state_ = State::kArrayN;
        return Status::Ok();
      }
      return Status(
          kBadReq,
          fmt::format("expect '\r' after length of the array instead of '{}'",
                      ch));
    case State::kArrayN:
      if (ch == '\n') {
        state_ = State::kStr;
        str_parser_.Reset();
        return Status::Ok();
      }
      return Status(kBadReq, fmt::format("expect '\n' after array "
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
      strs_.emplace_back(std::move(str_parser_.MoveOut()));
      str_parser_.Reset();
      str_count_++;
      if (str_count_ == array_len_) {
        *completed = true;
      }
      return Status::Ok();
  }
  return Status::Ok();
}

}  // namespace mydss

#endif  // MYDSS_INCLUDE_RESP_REQ_PARSER_HPP_
