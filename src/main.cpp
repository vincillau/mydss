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

#include <arg.hpp>
#include <config.hpp>
#include <db/inst.hpp>
#include <help.hpp>
#include <iostream>
#include <net/loop.hpp>
#include <nlohmann/json.hpp>
#include <server/server.hpp>
#include <vector>
#include <version.hpp>

using fmt::print;
using mydss::Arg;
using mydss::Config;
using mydss::kHelpText;
using mydss::db::Inst;
using mydss::net::Loop;
using mydss::server::Server;
using nlohmann::json;
using std::ifstream;
using std::shared_ptr;
using std::string;
using std::vector;

static void InitLogger() {
  spdlog::set_level(spdlog::level::debug);
  spdlog::flush_on(spdlog::level::debug);
}

int main(int argc, char** argv) {
  InitLogger();

  Arg arg;
  auto status = arg.Parse(argc, argv);
  if (status.error()) {
    SPDLOG_CRITICAL("{}", status.ToString());
    return EXIT_FAILURE;
  }

  if (arg.help()) {
    print("{}", kHelpText);
    return 0;
  } else if (arg.version()) {
    print("version: {}\n", MYDSS_VERSION);
    return 0;
  }

  auto config = Config::Default();
  if (!arg.conf_file().empty()) {
    status = Config::Load(arg.conf_file(), config);
    if (status.error()) {
      SPDLOG_CRITICAL("{}", status.ToString());
      return EXIT_FAILURE;
    }
  }

  Inst::Init(config.db().db_num());
  auto loop = Loop::New();

  vector<shared_ptr<Server>> servers;
  servers.reserve(config.server().size());
  for (const auto& sc : config.server()) {
    auto server = Server::New(loop, sc);
    auto status = server->Start();
    if (status.error()) {
      SPDLOG_CRITICAL("{}", status.ToString());
      return EXIT_FAILURE;
    }
    servers.push_back(server);
  }

  loop->Run();

  return 0;
}
