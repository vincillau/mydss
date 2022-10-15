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

#include <cmd/connection.hpp>
#include <db/inst.hpp>
#include <util/str.hpp>

using fmt::format;
using mydss::db::Inst;
using mydss::proto::BulkStringPiece;
using mydss::proto::ErrorPiece;
using mydss::proto::IntegerPiece;
using mydss::proto::NullPiece;
using mydss::proto::Req;
using mydss::proto::Resp;
using mydss::proto::SimpleStringPiece;
using mydss::util::StrLower;
using mydss::util::StrToI64;
using std::make_shared;

namespace mydss::cmd {

void Connection::Client(const Req& req, Resp& resp) {
  if (req.pieces().size() == 1) {
    resp.piece() = make_shared<ErrorPiece>(
        "wrong number of arguments for 'client' command");
    return;
  }

  auto sub_cmd_name = req.pieces()[1];
  StrLower(sub_cmd_name);
  if (sub_cmd_name == "getname") {
    ClientGetName(req, resp);
    return;
  }
  if (sub_cmd_name == "id") {
    ClientId(req, resp);
    return;
  }
  if (sub_cmd_name == "setname") {
    ClientSetName(req, resp);
    return;
  }
  resp.piece() = make_shared<ErrorPiece>(
      format("unknown subcommand '{}'. Try CLIENT HELP.", req.pieces()[1]));
}

void Connection::ClientGetName(const Req& req, Resp& resp) {
  if (req.pieces().size() != 2) {
    resp.piece() = make_shared<ErrorPiece>(
        "wrong number of arguments for 'client|getname' command");
    return;
  }

  const auto& client_name = req.client()->name();
  if (client_name.empty()) {
    resp.piece() = make_shared<NullPiece>();
    return;
  }
  resp.piece() = make_shared<BulkStringPiece>(client_name);
}

void Connection::ClientId(const Req& req, Resp& resp) {
  if (req.pieces().size() != 2) {
    resp.piece() = make_shared<ErrorPiece>(
        "wrong number of arguments for 'client|getname' command");
    return;
  }
  resp.piece() = make_shared<IntegerPiece>(req.client()->id());
}

void Connection::ClientSetName(const Req& req, Resp& resp) {
  if (req.pieces().size() != 3) {
    resp.piece() = make_shared<ErrorPiece>(
        "wrong number of arguments for 'client|getname' command");
    return;
  }
  req.client()->set_name(req.pieces()[2]);
  resp.piece() = make_shared<SimpleStringPiece>("OK");
}

void Connection::Echo(const proto::Req& req, proto::Resp& resp) {
  if (req.pieces().size() != 2) {
    resp.piece() =
        make_shared<ErrorPiece>("wrong number of arguments for 'echo' command");
    return;
  }
  resp.piece() = make_shared<BulkStringPiece>(req.pieces()[1]);
}

void Connection::Ping(const Req& req, Resp& resp) {
  if (req.pieces().size() > 2) {
    resp.piece() =
        make_shared<ErrorPiece>("wrong number of arguments for 'ping' command");
    return;
  }

  if (req.pieces().size() == 1) {
    resp.piece() = make_shared<SimpleStringPiece>("PONG");
    return;
  }

  resp.piece() = make_shared<BulkStringPiece>(req.pieces()[1]);
}

void Connection::Quit(const Req& req, Resp& resp) {
  resp.close() = true;
  resp.piece() = make_shared<SimpleStringPiece>("OK");
}

void Connection::Select(const Req& req, Resp& resp) {
  if (req.pieces().size() != 2) {
    resp.piece() = make_shared<ErrorPiece>(
        "wrong number of arguments for 'select' command");
    return;
  }

  int64_t index = 0;
  bool ok = StrToI64(req.pieces()[1], &index);
  if (!ok) {
    resp.piece() =
        make_shared<ErrorPiece>("value is not an integer or out of range");
    return;
  }

  ok = Inst::GetInst()->Select(index);
  if (ok) {
    resp.piece() = make_shared<SimpleStringPiece>("OK");
    return;
  }
  resp.piece() = make_shared<ErrorPiece>("DB index is out of range");
}

}  // namespace mydss::cmd
