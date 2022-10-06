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

#include <spdlog/spdlog.h>

#include <db/inst.hpp>
#include <net/addr.hpp>
#include <net/loop.hpp>
#include <net/server.hpp>

using mydss::db::Inst;
using mydss::net::Addr;
using mydss::net::Loop;
using mydss::net::Server;

int main() {
  spdlog::set_level(spdlog::level::debug);
  spdlog::flush_on(spdlog::level::debug);

  Inst::Init(16);

  auto loop = Loop::New();

  Addr addr("127.0.0.1", 6380);
  auto server = Server::New(loop, addr);
  server->Start();

  loop->Run();

  return 0;
}
