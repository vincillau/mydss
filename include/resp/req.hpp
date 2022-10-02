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

#ifndef MYDSS_INCLUDE_RESP_REQ_HPP_
#define MYDSS_INCLUDE_RESP_REQ_HPP_

#include <fmt/ostream.h>

#include <status.hpp>
#include <vector>

#include "piece.hpp"

namespace mydss {

// Redis 请求
class Req {
 public:
  [[nodiscard]] const auto& pieces() const { return pieces_; }
  [[nodiscard]] auto& pieces() { return pieces_; }

 private:
  std::vector<BulkStringPiece> pieces_;
};

inline static std::ostream& operator<<(std::ostream& os, const Req& req) {
  os << '(';
  for (size_t i = 0; i < req.pieces().size(); i++) {
    os << '\'' << req.pieces()[i].value() << '\'';
    if (i != req.pieces().size() - 1) {
      os << ", ";
    }
  }
  os << ')';
  return os;
}

}  // namespace mydss

#endif  // MYDSS_INCLUDE_RESP_REQ_HPP_
