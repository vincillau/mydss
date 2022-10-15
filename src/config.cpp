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

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <config.hpp>
#include <err/errno.hpp>
#include <nlohmann/json.hpp>
#include <util/str.hpp>

using fmt::format;
using mydss::err::ErrnoStr;
using mydss::err::kEof;
using mydss::err::kInvalidConfig;
using mydss::err::kJsonParseErr;
using mydss::err::Status;
using mydss::net::InetType;
using mydss::util::StrLower;
using nlohmann::json;
using std::ifstream;
using std::string;
using std::vector;

namespace mydss {

// 读取路径为 path 的文件，将读取到的内容存储在 data 中
static Status ReadFile(const string& path, string& data);
static Status LoadServerItem(const json& item, size_t index,
                             ServerConfig& result);

Status ServerConfig::Load(const json& json, vector<ServerConfig>& result) {
  auto server = json["server"];
  if (server.is_null()) {
    result = {};
    return Status::Ok();
  }
  if (!server.is_array()) {
    return {kInvalidConfig, "the 'server' field must be a array"};
  }

  vector<ServerConfig> scs;
  for (size_t i = 0; i < server.size(); i++) {
    ServerConfig sc;
    LoadServerItem(server[i], i, sc);
    scs.push_back(std::move(sc));
  }

  result = std::move(scs);
  return Status::Ok();
}

Status DbConfig::Load(const json& json, DbConfig& result) {
  auto db = json["db"];
  if (db.is_null()) {
    result = {};
    return Status::Ok();
  }
  if (!db.is_object()) {
    return {kInvalidConfig, "the 'db' field must be a object"};
  }

  auto db_num = db["db_num"];
  DbConfig dc;
  if (!db_num.is_null()) {
    if (!db_num.is_number_integer()) {
      return {kInvalidConfig, "the 'db.db_num' field must be a integer"};
    }
    if (db_num <= 0 || db_num > 255) {
      return {kInvalidConfig,
              "the 'db.db_num' field must be in the range of 1-255"};
    }
    dc.set_db_num(db_num);
  }

  result = std::move(dc);
  return Status::Ok();
}

Status Config::Load(const string& conf_file, Config& config) {
  string conf_str;
  auto status = ReadFile(conf_file, conf_str);
  if (status.error()) {
    return status;
  }

  json conf_json;
  try {
    conf_json = json::parse(conf_str);
  } catch (const json::parse_error& e) {
    return {kJsonParseErr, ErrnoStr()};
  }

  if (!conf_json.is_object()) {
    return {kInvalidConfig, "config must be a object"};
  }

  status = ServerConfig::Load(conf_json, config.server());
  if (status.error()) {
    return status;
  }
  status = DbConfig::Load(conf_json, config.db());
  if (status.error()) {
    return status;
  }

  return Status::Ok();
}

Status ReadFile(const string& path, string& data) {
  struct stat file_stat;
  int ret = stat(path.c_str(), &file_stat);
  if (ret == -1) {
    return {errno, ErrnoStr()};
  }

  int fd = open(path.c_str(), O_RDONLY);
  if (fd == -1) {
    return {errno, ErrnoStr()};
  }

  size_t file_size = file_stat.st_size;
  data.resize(file_size);
  auto nbytes = read(fd, data.data(), file_size);
  if (nbytes == -1) {
    return {errno, ErrnoStr()};
  }
  if (nbytes < file_size) {
    return {kEof, ErrnoStr()};
  }

  close(fd);
  return Status::Ok();
}

Status LoadServerItem(const json& item, size_t index, ServerConfig& result) {
  if (!item.is_object()) {
    return {kInvalidConfig,
            format("the 'server[{}]' field must be a object", index)};
  }

  auto type = item["type"];
  if (!type.is_null()) {
    if (!type.is_string()) {
      return {kInvalidConfig,
              format("the 'server[{}].type' field must be a string", index)};
    }

    string type_str = type;
    StrLower(type_str);
    if (type_str == "ipv4") {
      result.set_type(InetType::kIPv4);
    } else if (type_str == "ipv6") {
      result.set_type(InetType::kIPv6);
    } else {
      return {kInvalidConfig,
              format("the 'server[{}].type' field must be 'IPv4' or 'IPv6'",
                     index)};
    }
  }

  auto ip = item["ip"];
  if (!ip.is_null()) {
    if (!type.is_string()) {
      return {kInvalidConfig,
              format("the 'server[{}].ip' field must be a string", index)};
    }
    result.set_ip(ip);
  }

  auto port = item["port"];
  if (!port.is_null()) {
    if (!port.is_number_integer()) {
      return {kInvalidConfig,
              format("the 'server[{}].port' field must be a integer", index)};
    }
    if (port <= 0 || port > 65535) {
      return {
          kInvalidConfig,
          format("the 'server[{}].port' field must be in the range of 1-65535",
                 index)};
    }
    result.set_port(port);
  }

  return Status::Ok();
}

}  // namespace mydss
