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

#ifndef MYDSS_INCLUDE_CMD_GENERIC_HPP_
#define MYDSS_INCLUDE_CMD_GENERIC_HPP_

#include <proto/piece.hpp>
#include <proto/req.hpp>

namespace mydss::cmd {

class Generic {
 public:
  static void Del(const proto::Req& req, std::shared_ptr<proto::Piece>& resp);
  static void Exists(const proto::Req& req,
                     std::shared_ptr<proto::Piece>& resp);
  static void Expire(const proto::Req& req,
                     std::shared_ptr<proto::Piece>& resp);
  static void ExpireAt(const proto::Req& req,
                       std::shared_ptr<proto::Piece>& resp);
  static void Object(const proto::Req& req,
                     std::shared_ptr<proto::Piece>& resp);
  static void ObjectEncoding(const proto::Req& req,
                             std::shared_ptr<proto::Piece>& resp);
  static void ObjectIdleTime(const proto::Req& req,
                             std::shared_ptr<proto::Piece>& resp);
  static void ObjectRefCount(const proto::Req& req,
                             std::shared_ptr<proto::Piece>& resp);
  static void Persist(const proto::Req& req,
                      std::shared_ptr<proto::Piece>& resp);
  static void PExpire(const proto::Req& req,
                      std::shared_ptr<proto::Piece>& resp);
  static void PExpireAt(const proto::Req& req,
                        std::shared_ptr<proto::Piece>& resp);
  static void Pttl(const proto::Req& req, std::shared_ptr<proto::Piece>& resp);
  static void Rename(const proto::Req& req,
                     std::shared_ptr<proto::Piece>& resp);
  static void RenameNX(const proto::Req& req,
                       std::shared_ptr<proto::Piece>& resp);
  static void Touch(const proto::Req& req, std::shared_ptr<proto::Piece>& resp);
  static void Ttl(const proto::Req& req, std::shared_ptr<proto::Piece>& resp);
  static void Type(const proto::Req& req, std::shared_ptr<proto::Piece>& resp);
};

}  // namespace mydss::cmd

#endif  // MYDSS_INCLUDE_CMD_GENERIC_HPP_
