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

#ifndef MYDSS_INCLUDE_NET_CONN_HPP_
#define MYDSS_INCLUDE_NET_CONN_HPP_

#include <asio.hpp>
#include <memory>
#include <resp/req_parser.hpp>

#include "send_req.hpp"

namespace mydss {

class Conn : public std::enable_shared_from_this<Conn> {
 public:
  static constexpr std::size_t kBufSize = 4096;

  Conn(std::shared_ptr<asio::ip::tcp::socket> sock)
      : sock_(sock), remote_(sock_->remote_endpoint()) {}

  std::string GetRemoteStr() {
    return fmt::format("{}:{}", remote_.address().to_string(), remote_.port());
  }

  void Recv(std::size_t buf_size);

 private:
  void Send(std::shared_ptr<SendReq> send_req);
  void HandleRecvErr(const asio::error_code& err);

  void SendResult(const Piece& piece);
  void SendSimpleString(std::string str);
  void SendError(std::string str);
  void SendBulkString(std::string str);
  void SendInteger(std::uint64_t u64);

  static void OnRecv(std::shared_ptr<Conn> conn, std::shared_ptr<char*> buf,
                     const asio::error_code& err, std::size_t nrecv);
  static void AfterSend(std::shared_ptr<Conn> conn,
                        std::shared_ptr<SendReq> send_req,
                        const asio::error_code& err, std::size_t nsend);

 private:
  std::shared_ptr<asio::ip::tcp::socket> sock_;
  // 远程关闭连接后无法获取远程的地址，所以提前保存
  asio::ip::tcp::endpoint remote_;
  ReqParser parser_;
};

inline void Conn::SendResult(const Piece& piece) {
  auto send_req =
      std::make_shared<SendReq>(piece.ToString(), SendReq::Action::kRecv);
  Send(send_req);
}

inline void Conn::SendSimpleString(std::string str) {
  SimpleStringPiece piece(std::move(str));
  SendResult(piece);
}

inline void Conn::SendError(std::string str) {
  ErrorPiece piece(std::move(str));
  SendResult(piece);
}

inline void Conn::SendBulkString(std::string str) {
  BulkStringPiece piece(std::move(str));
  SendResult(piece);
}

inline void Conn::SendInteger(std::uint64_t u64) {
  IntegerPiece piece(u64);
  SendResult(piece);
}

}  // namespace mydss

#endif  // MYDSS_INCLUDE_NET_CONN_HPP_
