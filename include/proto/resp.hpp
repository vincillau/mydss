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

#ifndef MYDSS_INCLUDE_PROTO_RESP_HPP_
#define MYDSS_INCLUDE_PROTO_RESP_HPP_

#include <memory>

#include "piece.hpp"

namespace mydss::proto {

class Resp {
 public:
  [[nodiscard]] const auto& piece() const { return piece_; }
  [[nodiscard]] auto& piece() { return piece_; }

  [[nodiscard]] const auto& close() const { return close_; }
  [[nodiscard]] auto& close() { return close_; }

 private:
  std::shared_ptr<proto::Piece> piece_;
  bool close_ = false;  // 发送完请求后关闭连接
};

}  // namespace mydss::proto

#endif  // MYDSS_INCLUDE_PROTO_RESP_HPP_
