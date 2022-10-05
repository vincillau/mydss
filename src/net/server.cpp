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

using mydss::err::ErrnoStr;
using mydss::err::kEof;
using mydss::err::kOk;
using mydss::util::Buf;
using std::shared_ptr;
using std::string;
using std::placeholders::_1;
using std::placeholders::_2;

namespace mydss::net {

static constexpr int kBacklog = 512;
static constexpr int kBufSize = 2048;

void Server::Start() {
  acceptor_->Bind(addr_);
  acceptor_->Listen(kBacklog);
  auto conn = Conn::New(loop_);
  acceptor_->AsyncAccept(conn,
                         bind(&Server::OnAccept, shared_from_this(), conn));
}

void Server::OnAccept(shared_ptr<Server> server, shared_ptr<Conn> conn) {
  auto buf = shared_ptr<char>(new char[kBufSize]);
  conn->AsyncRecv(Buf(buf.get(), kBufSize),
                  bind(&Server::OnRecv, conn, buf, _1, _2));

  auto new_conn = Conn::New(server->loop_);
  server->acceptor_->AsyncAccept(conn, bind(&Server::OnAccept, server, conn));
}

void Server::OnRecv(shared_ptr<Conn> conn, shared_ptr<char> buf, int err,
                    int nbytes) {
  if (err == kEof) {
    SPDLOG_DEBUG("connection was closed by peer");
    conn->Close();
    return;
  }

  if (err != kOk) {
    SPDLOG_CRITICAL("receive failed, reason='{}'", ErrnoStr());
    exit(EXIT_FAILURE);
  }

  SPDLOG_DEBUG("received data: '{}'", string(buf.get(), nbytes));

  conn->AsyncRecv(Buf(buf.get(), kBufSize),
                  bind(&Server::OnRecv, conn, buf, _1, _2));
}

}  // namespace mydss::net
