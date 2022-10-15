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

#include <module/api.hpp>

namespace mydss::cmd {

class Generic {
 public:
  static void Del(module::Ctx& ctx, module::Req req);
  static void Exists(module::Ctx& ctx, module::Req req);
  static void Expire(module::Ctx& ctx, module::Req req);
  static void ExpireAt(module::Ctx& ctx, module::Req req);
  static void Object(module::Ctx& ctx, module::Req req);
  static void ObjectEncoding(module::Ctx& ctx, module::Req req);
  static void ObjectIdleTime(module::Ctx& ctx, module::Req req);
  static void ObjectRefCount(module::Ctx& ctx, module::Req req);
  static void Persist(module::Ctx& ctx, module::Req req);
  static void PExpire(module::Ctx& ctx, module::Req req);
  static void PExpireAt(module::Ctx& ctx, module::Req req);
  static void PTtl(module::Ctx& ctx, module::Req req);
  static void Rename(module::Ctx& ctx, module::Req req);
  static void RenameNx(module::Ctx& ctx, module::Req req);
  static void Touch(module::Ctx& ctx, module::Req req);
  static void Ttl(module::Ctx& ctx, module::Req req);
  static void Type(module::Ctx& ctx, module::Req req);
};

}  // namespace mydss::cmd

#endif  // MYDSS_INCLUDE_CMD_GENERIC_HPP_
