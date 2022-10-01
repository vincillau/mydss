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

#ifndef MYDSS_INCLUDE_NET_SEND_REQ_HPP_
#define MYDSS_INCLUDE_NET_SEND_REQ_HPP_

#include <string>

namespace mydss {

class SendReq {
 public:
  // 发送完 SendReq 后的动作
  enum class Action {
    kDoNothing,  // 不做任何动作
    kRecv,       // 接收数据
    kClose       // 关闭连接
  };

  SendReq(std::string data, Action action)
      : data_(std::move(data)), action_(action) {}

  [[nodiscard]] const auto& data() const { return data_; }
  [[nodiscard]] auto action() const { return action_; }

 private:
  std::string data_;
  Action action_;
};

}  // namespace mydss

#endif  // MYDSS_INCLUDE_NET_SEND_REQ_HPP_
