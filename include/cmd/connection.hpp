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

#include <proto/req.hpp>
#include <proto/resp.hpp>

namespace mydss::cmd {

class Connection {
 public:
  static void Client(const proto::Req& req, proto::Resp& resp);
  static void ClientGetName(const proto::Req& req, proto::Resp& resp);
  static void ClientId(const proto::Req& req, proto::Resp& resp);
  static void ClientSetName(const proto::Req& req, proto::Resp& resp);
  static void Echo(const proto::Req& req, proto::Resp& resp);
  static void Ping(const proto::Req& req, proto::Resp& resp);
  static void Quit(const proto::Req& req, proto::Resp& resp);
  static void Select(const proto::Req& req, proto::Resp& resp);
};

}  // namespace mydss::cmd

#endif  // MYDSS_INCLUDE_CMD_CONNECTION_HPP_
