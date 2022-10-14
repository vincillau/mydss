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

#ifndef MYDSS_INCLUDE_SERVER_SESSION_HPP_
#define MYDSS_INCLUDE_SERVER_SESSION_HPP_

#include <memory>
#include <net/conn.hpp>
#include <proto/parser.hpp>
#include <proto/piece.hpp>

#include "client.hpp"

namespace mydss::server {

class Session : public std::enable_shared_from_this<Session> {
 public:
  [[nodiscard]] static auto New(std::shared_ptr<net::Conn> conn) {
    return std::shared_ptr<Session>(new Session(conn));
  }

  void Start();

  [[nodiscard]] auto client() const { return client_; }

 private:
  Session(std::shared_ptr<net::Conn> conn) : conn_(conn) {}

  void Send(std::shared_ptr<proto::Piece> piece, bool close = false);

  static void OnRecv(std::shared_ptr<Session> session, util::Slice slice,
                     err::Status status, int nbytes);
  static void OnSend(std::shared_ptr<Session> session, util::Slice slice,
                     bool close, err::Status status);

 private:
  std::shared_ptr<net::Conn> conn_;  // 与客户端的连接
  std::shared_ptr<Client> client_;  // 表示客户端，存储与客户端的相关信息
  proto::ReqParser parser_;
};

}  // namespace mydss::server

#endif  // MYDSS_INCLUDE_SERVER_SESSION_HPP_
