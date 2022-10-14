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
#include <server/server.hpp>
#include <server/session.hpp>

using mydss::err::Status;
using mydss::net::Acceptor;
using mydss::net::Conn;
using mydss::net::EndPoint;
using std::shared_ptr;

namespace mydss::server {

Status Server::Start() {
  EndPoint ep(config_.type(), config_.ip(), config_.port());
  acceptor_ = Acceptor::New(loop_, std::move(ep));
  auto status = acceptor_->Start(config_.backlog());
  if (status.error()) {
    return status;
  }

  auto conn = Conn::New();
  acceptor_->AsyncAccept(conn,
                         bind(&Server::OnAccept, shared_from_this(), conn));
  return Status::Ok();
}

void Server::OnAccept(shared_ptr<Server> server, shared_ptr<Conn> conn) {
  auto session = Session::New(conn);
  session->Start();

  auto new_conn = Conn::New();
  server->acceptor_->AsyncAccept(new_conn,
                                 bind(&Server::OnAccept, server, new_conn));
}

}  // namespace mydss::server
