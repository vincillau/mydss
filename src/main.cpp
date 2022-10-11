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

#include <getopt.h>
#include <spdlog/spdlog.h>

#include <config.hpp>
#include <db/inst.hpp>
#include <iostream>
#include <net/addr.hpp>
#include <net/loop.hpp>
#include <net/server.hpp>
#include <nlohmann/json.hpp>
#include <version.hpp>

using fmt::print;
using mydss::Config;
using mydss::db::Inst;
using mydss::net::Addr;
using mydss::net::Loop;
using mydss::net::Server;
using nlohmann::json;
using std::cout;
using std::endl;
using std::ifstream;
using std::string;

static const char* kHelpText =
    "Usage:\n"
    "  myping [options]\n"
    "\n"
    "Options:\n"
    "  -c, --config file    specify configure file path\n"
    "  --version            show version\n"
    "  --help               show help text\n";

static constexpr int kHelpVal = 256;
static constexpr int kVersionVal = 257;

static bool LoadConfig(const string& conf_path, Config& config) {
  assert(!conf_path.empty());

  ifstream ifs(conf_path);
  if (!ifs.is_open()) {
    print("open config file '{}' failed\n", conf_path);
    return false;
  }

  json json_config;
  try {
    json_config = json::parse(ifs);
  } catch (const json::parse_error& e) {
    print("parse config file failed: '{}'\n", e.what());
    return false;
  }

  if (!json_config.is_object()) {
    print("config must be a object\n");
    return false;
  }

  json ip = json_config["ip"];
  if (!ip.is_null()) {
    if (ip.is_string()) {
      config.set_ip(ip);
    } else {
      print("ip must be a string\n");
      return false;
    }
  }

  json port = json_config["port"];
  if (!port.is_null()) {
    if (port.is_number_integer()) {
      if (port <= 0 || port >= 65535) {
        print("port must be greater than 0 and less than 65536\n");
        return false;
      }
      config.set_port(port);
    } else {
      print("port must be a integer\n");
      return false;
    }
  }

  json db_num = json_config["db_num"];
  if (!db_num.is_null()) {
    if (db_num.is_number_integer()) {
      if (db_num <= 0) {
        print("db_num must be greater than 0\n");
        return false;
      }
      config.set_db_num(db_num);
    } else {
      print("db_num must be a integer\n");
      return false;
    }
  }

  return true;
}

int main(int argc, char** argv) {
  std::string conf_path;
  opterr = 1;

  option opts[3];
  opts[0].name = "help";
  opts[0].has_arg = no_argument;
  opts[0].flag = nullptr;
  opts[0].val = kHelpVal;
  opts[1].name = "version";
  opts[1].has_arg = no_argument;
  opts[1].flag = nullptr;
  opts[1].val = kVersionVal;
  opts[2].name = nullptr;
  opts[2].has_arg = no_argument;
  opts[2].flag = nullptr;
  opts[2].val = 0;

  for (;;) {
    int val = getopt_long(argc, argv, "c:", opts, nullptr);
    if (val == -1) {
      break;
    }

    switch (val) {
      case 'c':
        conf_path = optarg;
        break;
      case kHelpVal:
        cout << kHelpText << endl;
        return 0;
      case kVersionVal:
        cout << "version: " << MYDSS_VERSION << endl;
        return 0;
      case '?':
        return 1;
    }
  }

  spdlog::set_level(spdlog::level::debug);
  spdlog::flush_on(spdlog::level::debug);

  Config config;
  if (!conf_path.empty()) {
    bool ok = LoadConfig(conf_path, config);
    if (!ok) {
      return 1;
    }
    SPDLOG_INFO("loaded config file");
  }

  SPDLOG_DEBUG("ip='{}', port='{}', db_num='{}'", config.ip(), config.port(),
               config.db_num());

  Inst::Init(config.db_num());
  auto loop = Loop::New();
  Addr addr(config.ip(), config.port());
  auto server = Server::New(loop, addr);
  server->Start();
  loop->Run();

  return 0;
}
