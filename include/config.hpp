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

#ifndef MYDSS_INCLUDE_CONFIG_HPP_
#define MYDSS_INCLUDE_CONFIG_HPP_

#include <cstdint>
#include <err/status.hpp>
#include <net/inet.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace mydss {

// 服务器配置
class ServerConfig {
 public:
  [[nodiscard]] auto type() const { return type_; }
  [[nodiscard]] const auto& ip() const { return ip_; }
  [[nodiscard]] auto port() const { return port_; }
  [[nodiscard]] auto backlog() const { return backlog_; }

  void set_type(net::InetType type) { type_ = type; }
  void set_ip(std::string ip) { ip_ = std::move(ip); }
  void set_port(uint16_t port) { port_ = port; }
  void set_backlog(int backlog) { backlog_ = backlog; }

  // 从 json 中加载服务器配置，并将结果存储到 result
  static err::Status Load(const nlohmann::json& json,
                          std::vector<ServerConfig>& result);

 private:
  // 地址类型，IPv4 或 IPv6
  net::InetType type_ = net::InetType::kIPv4;
  // 监听的 IP 地址
  std::string ip_ = "127.0.0.1";
  // 监听的端口
  uint16_t port_ = 6379;
  // listen 系统调用的 backlog 参数
  int backlog_ = 512;
};

// 数据库配置
class DbConfig {
 public:
  [[nodiscard]] uint8_t db_num() const { return db_num_; }
  void set_db_num(uint8_t db_num) { db_num_ = db_num; }

  // 从 json 中加载服务器配置，并将结果存储到 result
  [[nodiscard]] static err::Status Load(const nlohmann::json& json,
                                        DbConfig& result);

 private:
  uint8_t db_num_ = 16;  // 数据库的数目
};

// MyDSS 配置
class Config {
 public:
  [[nodiscard]] const auto& server() const { return server_; }
  [[nodiscard]] auto& server() { return server_; }

  [[nodiscard]] const auto& db() const { return db_; }
  [[nodiscard]] auto& db() { return db_; }

  // 返回默认配置
  // 默认配置为：
  // 1. 服务器监听 127.0.0.0:6379，backlog=512
  // 2. 数据库数目为 16
  [[nodiscard]] static Config Default() {
    Config config;
    config.server_.push_back({});
    return config;
  }

  // 从配置文件中加载配置
  // config_file 为配置文件的路径，解析到的配置存储在 config 中
  // 若成功，返回 Status::Ok()；若失败，返回错误信息
  [[nodiscard]] static err::Status Load(const std::string& conf_file,
                                        Config& config);

 private:
  std::vector<ServerConfig> server_;  // 服务器配置，支持同时监听多个地址
  DbConfig db_;                       // 数据库配置
};

}  // namespace mydss

#endif  // MYDSS_INCLUDE_CONFIG_HPP_
