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

#include <err/code.hpp>
#include <err/errno.hpp>
#include <net/server.hpp>
#include <net/session.hpp>

using std::shared_ptr;

namespace mydss::net {

static constexpr int kBacklog = 512;

void Server::Start() {
  acceptor_->Bind(addr_);
  acceptor_->Listen(kBacklog);
  auto conn = Conn::New(loop_);
  acceptor_->AsyncAccept(conn,
                         bind(&Server::OnAccept, shared_from_this(), conn));
}

void Server::OnAccept(shared_ptr<Server> server, shared_ptr<Conn> conn) {
  auto session = Session::New(conn);
  session->Start();

  auto new_conn = Conn::New(server->loop_);
  server->acceptor_->AsyncAccept(new_conn,
                                 bind(&Server::OnAccept, server, new_conn));
}

}  // namespace mydss::net
