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

#ifndef MYDSS_INCLUDE_ERR_CODE_HPP_
#define MYDSS_INCLUDE_ERR_CODE_HPP_

#include <cerrno>

namespace mydss::err {

// MyDSS 错误码
// 错误码的具体值可能会改变，所以不能依赖于错误码的具体值
static constexpr int kOk = 0;  // 成功

// 1-1000 保留给 errno

static constexpr int kUnknown = 1001;        // 未知错误
static constexpr int kEof = 1002;            // 读取到 EOF
static constexpr int kBadReq = 1003;         // 格式错误的请求
static constexpr int kJsonParseErr = 1004;   // JSON 解析错误
static constexpr int kInvalidConfig = 1005;  // 无效的配置
static constexpr int kInvalidArgs = 1006;    // 无效的参数
static constexpr int kInvalidAddr = 1007;    // 无效的地址

}  // namespace mydss::err

#endif  // MYDSS_INCLUDE_ERR_CODE_HPP_
