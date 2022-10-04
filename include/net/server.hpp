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

#ifndef MYDSS_INCLUDE_NET_SERVER_HPP_
#define MYDSS_INCLUDE_NET_SERVER_HPP_

#include <asio.hpp>
#include <cstdint>
#include <memory>
#include <status.hpp>
#include <string>

namespace mydss {

class Server {
 public:
  Server(std::string ip, std::uint16_t port,
         std::shared_ptr<asio::io_context> ctx)
      : ip_(std::move(ip)), port_(port), ctx_(ctx) {}

  void Run();

 private:
  void OnAccept(std::shared_ptr<asio::ip::tcp::socket> sock,
                const asio::error_code& err);

 private:
  std::string ip_;
  std::uint16_t port_;
  std::shared_ptr<asio::io_context> ctx_;
  std::shared_ptr<asio::ip::tcp::acceptor> acceptor_;
};

}  // namespace mydss

#endif  // MYDSS_INCLUDE_NET_SERVER_HPP_
