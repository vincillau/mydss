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

#ifndef MYDSS_INCLUDE_SERVER_PARSER_HPP_
#define MYDSS_INCLUDE_SERVER_PARSER_HPP_

#include <err/status.hpp>
#include <module/req.hpp>
#include <vector>

namespace mydss::server {

// 解析 bulk string
// BulkStringParser 从不在内部调用 Reset，因此使用者必须在需要重置
// BulkStringParser 的状态时调用 Reset
class BulkStringParser {
 public:
  // 重置解析器的内部状态
  void Reset();

  // 进行一次状态转移，如果解析出一个完整的字符串，completed 设置为 true，否则为
  // false
  [[nodiscard]] err::Status Step(char ch, bool* completed);

  // 将解析得到的字符串移出
  // 再次使用该解析器时应该调用 Reset
  [[nodiscard]] std::string MoveOut() { return std::move(value_); }

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
  State state_;          // 解析器的内部状态
  std::string value_;    // bulk string  的值
  uint64_t target_len_;  // 期待的 bulk string 长度
  uint64_t cur_len_;     // 当前已经读取到的 bulk string 的长度
};

// 解析请求
// 与 BulkStringParser 不同，ReqParser 在解析完一个请求后会自动重置状态
class ReqParser {
 public:
  ReqParser() { Reset(); }

  // 解析一段数据
  [[nodiscard]] err::Status Parse(const char* buf, size_t len,
                                  std::vector<module::Req>& reqs);

 private:
  enum class State {
    kArrayChar,  // 期待下一个接收的字符为表示数组的字符，即 '*'
    kArrayLenFirstNum,  // 期待接收到数组长度的第一个数字字符
    kArrayLen,          // 正在接收数组长度
    kArrayN,            // 期待接收数组长度后的 \n
    kStr                // 接收字符串
  };

  // 重置解析器的内部状态
  void Reset();

  // 进行一次状态转移，如果解析出一个完整的请求，completed 设置为 true，否则为
  // false
  [[nodiscard]] err::Status Step(char ch, bool* completed);

 private:
  State state_;                  // 解析器的内部状态
  module::Req req_;              // 正在解析的请求
  uint64_t array_len_;           // 数组的长度
  BulkStringParser str_parser_;  // 字符串部分的解析器
};

}  // namespace mydss::server

#endif  // MYDSS_INCLUDE_SERVER_PARSER_HPP_
