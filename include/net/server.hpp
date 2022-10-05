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

#include <memory>

#include "acceptor.hpp"
#include "addr.hpp"
#include "loop.hpp"

namespace mydss::net {

class Server : public std::enable_shared_from_this<Server> {
 public:
  static auto New(std::shared_ptr<Loop> loop, const Addr& addr) {
    return std::shared_ptr<Server>(new Server(loop, addr));
  }

  void Start();

 private:
  static void OnAccept(std::shared_ptr<Server> server,
                       std::shared_ptr<Conn> conn);

  static void OnRecv(std::shared_ptr<Conn> conn, std::shared_ptr<char> buf,
                     int err, int nbytes);

  Server(std::shared_ptr<Loop> loop, Addr addr)
      : loop_(loop), addr_(std::move(addr)) {
    acceptor_ = Acceptor::New(loop);
  }

 private:
  std::shared_ptr<Loop> loop_;
  std::shared_ptr<Acceptor> acceptor_;
  Addr addr_;
};

}  // namespace mydss::net

#endif  // MYDSS_INCLUDE_NET_SERVER_HPP_
