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

#ifndef MYDSS_INCLUDE_CMD_CONNECTION_HPP_
#define MYDSS_INCLUDE_CMD_CONNECTION_HPP_

#include <module/api.hpp>

namespace mydss::cmd {

class Connection {
 public:
  static void Client(module::Ctx& ctx, module::Req req);
  static void ClientGetName(module::Ctx& ctx, module::Req req);
  static void ClientId(module::Ctx& ctx, module::Req req);
  static void ClientSetName(module::Ctx& ctx, module::Req req);
  static void Echo(module::Ctx& ctx, module::Req req);
  static void Ping(module::Ctx& ctx, module::Req req);
  static void Quit(module::Ctx& ctx, module::Req req);
  static void Select(module::Ctx& ctx, module::Req req);
};

}  // namespace mydss::cmd

#endif  // MYDSS_INCLUDE_CMD_CONNECTION_HPP_
