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

#include <cstdlib>
#include <functional>
#include <net/conn.hpp>
#include <net/server.hpp>

using asio::ip::address;
using asio::ip::tcp;
using std::bind;
using std::error_code;
using std::make_shared;
using std::shared_ptr;
using std::system_error;
using std::placeholders::_1;

namespace mydss {

void Server::Run() {
  SPDLOG_INFO("bind on {}:{}", ip_, port_);

  SPDLOG_DEBUG("try to accept a new connection");
  tcp::endpoint ep(address::from_string(ip_), port_);

  try {
    acceptor_ = make_shared<tcp::acceptor>(*ctx_, ep);
  } catch (const system_error& e) {
    SPDLOG_CRITICAL("create acceptor error: `({}) {}`", e.code(), e.what());
    exit(EXIT_FAILURE);
  }

  auto sock = make_shared<tcp::socket>(*ctx_);
  acceptor_->async_accept(*sock, bind(&Server::OnAccept, this, sock, _1));

  ctx_->run();
}

void Server::OnAccept(shared_ptr<tcp::socket> sock, const error_code& err) {
  if (err) {
    SPDLOG_CRITICAL("accept error: `({}) {}`", err, err.message());
    exit(EXIT_FAILURE);
  }

  SPDLOG_DEBUG("accepted from {}:{}",
               sock->remote_endpoint().address().to_string(),
               sock->remote_endpoint().port());

  auto conn = make_shared<Conn>(sock);
  conn->Recv(Conn::kBufSize);

  SPDLOG_DEBUG("try to accept a new connection");
  auto new_sock = make_shared<tcp::socket>(*ctx_);
  acceptor_->async_accept(*new_sock,
                          bind(&Server::OnAccept, this, new_sock, _1));
}

}  // namespace mydss
