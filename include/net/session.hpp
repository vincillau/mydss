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

#ifndef MYDSS_INCLUDE_NET_SESSION_HPP_
#define MYDSS_INCLUDE_NET_SESSION_HPP_

#include <memory>
#include <proto/parser.hpp>
#include <proto/piece.hpp>

#include "client.hpp"
#include "conn.hpp"

namespace mydss::net {

class Session : public std::enable_shared_from_this<Session> {
 public:
  [[nodiscard]] static auto New(std::shared_ptr<Conn> conn) {
    return std::shared_ptr<Session>(new Session(conn));
  }

  void Start();

  [[nodiscard]] const auto& client() const { return client_; }
  [[nodiscard]] auto& client() { return client_; }

 private:
  Session(std::shared_ptr<Conn> conn) : conn_(conn) {}

  void Send(std::shared_ptr<proto::Piece> piece, bool close = false);

  static void OnRecv(std::shared_ptr<Session> session,
                     std::shared_ptr<char[]> buf, int err, int nbytes);
  static void OnSend(std::shared_ptr<Session> session,
                     std::shared_ptr<char[]> buf, bool close, int err,
                     int nbytes);

 private:
  std::shared_ptr<Conn> conn_;
  proto::ReqParser parser_;
  net::Client client_;
};

}  // namespace mydss::net

#endif  // MYDSS_INCLUDE_NET_SESSION_HPP_
