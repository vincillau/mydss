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

#ifndef MYDSS_INCLUDE_SERVER_SERVER_HPP_
#define MYDSS_INCLUDE_SERVER_SERVER_HPP_

#include <config.hpp>
#include <memory>
#include <net/acceptor.hpp>
#include <net/loop.hpp>

namespace mydss::server {

class Server : public std::enable_shared_from_this<Server> {
 public:
  static auto New(std::shared_ptr<net::Loop> loop, ServerConfig config) {
    return std::shared_ptr<Server>(new Server(loop, std::move(config)));
  }

  [[nodiscard]] err::Status Start();

 private:
  Server(std::shared_ptr<net::Loop> loop, ServerConfig config)
      : loop_(loop), config_(std::move(config)) {}

  static void OnAccept(std::shared_ptr<Server> server,
                       std::shared_ptr<net::Conn> conn);

 private:
  ServerConfig config_;
  std::shared_ptr<net::Loop> loop_;
  std::shared_ptr<net::Acceptor> acceptor_;
};

}  // namespace mydss::server

#endif  // MYDSS_INCLUDE_SERVER_SERVER_HPP_
