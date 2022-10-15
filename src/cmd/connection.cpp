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

using fmt::format;
using mydss::module::BulkStringPiece;
using mydss::module::Ctx;
using mydss::module::ErrorPiece;
using mydss::module::IntegerPiece;
using mydss::module::NullPiece;
using mydss::module::SimpleStringPiece;
using std::make_shared;
using std::string;
using std::vector;

namespace mydss::cmd {

static void StrLower(string& str) {
  for (char& ch : str) {
    if (ch >= 'A' && ch <= 'Z') {
      ch = ch - 'A' + 'a';
    }
  }
}

void Connection::Client(Ctx& ctx, vector<string> req) {
  if (req.size() == 1) {
    auto piece = make_shared<ErrorPiece>(
        "wrong number of arguments for 'client' command");
    ctx.Reply(piece);
    return;
  }

  auto sub_cmd_name = req[1];
  StrLower(sub_cmd_name);
  if (sub_cmd_name == "getname") {
    ClientGetName(ctx, std::move(req));
    return;
  }
  if (sub_cmd_name == "id") {
    ClientId(ctx, std::move(req));
    return;
  }
  if (sub_cmd_name == "setname") {
    ClientSetName(ctx, std::move(req));
    return;
  }
  auto piece = make_shared<ErrorPiece>(
      format("unknown subcommand '{}'. Try CLIENT HELP.", req[1]));
  ctx.Reply(piece);
}

void Connection::ClientGetName(Ctx& ctx, vector<string> req) {
  if (req.size() != 2) {
    auto piece = make_shared<ErrorPiece>(
        "wrong number of arguments for 'client|getname' command");
    ctx.Reply(piece);
    return;
  }

  const auto& client_name = ctx.GetClientName();
  if (client_name.empty()) {
    auto piece = make_shared<NullPiece>();
    ctx.Reply(piece);
    return;
  }
  auto piece = make_shared<BulkStringPiece>(client_name);
  ctx.Reply(piece);
}

void Connection::ClientId(Ctx& ctx, vector<string> req) {
  if (req.size() != 2) {
    auto piece = make_shared<ErrorPiece>(
        "wrong number of arguments for 'client|getname' command");
    ctx.Reply(piece);
    return;
  }
  auto piece = make_shared<IntegerPiece>(ctx.GetClientId());
  ctx.Reply(piece);
}

void Connection::ClientSetName(Ctx& ctx, vector<string> req) {
  if (req.size() != 3) {
    auto piece = make_shared<ErrorPiece>(
        "wrong number of arguments for 'client|getname' command");
    return;
  }
  ctx.SetClientName(req[2]);
  auto piece = make_shared<SimpleStringPiece>("OK");
  ctx.Reply(piece);
}

void Connection::Echo(Ctx& ctx, vector<string> req) {
  if (req.size() != 2) {
    auto piece =
        make_shared<ErrorPiece>("wrong number of arguments for 'echo' command");
    ctx.Reply(piece);
    return;
  }
  auto piece = make_shared<BulkStringPiece>(req[1]);
  ctx.Reply(piece);
}

void Connection::Ping(Ctx& ctx, vector<string> req) {
  if (req.size() > 2) {
    auto piece =
        make_shared<ErrorPiece>("wrong number of arguments for 'ping' command");
    ctx.Reply(piece);
    return;
  }

  if (req.size() == 1) {
    auto piece = make_shared<SimpleStringPiece>("PONG");
    ctx.Reply(piece);
    return;
  }

  auto piece = make_shared<BulkStringPiece>(req[1]);
  ctx.Reply(piece);
}

void Connection::Quit(Ctx& ctx, vector<string> req) {
  auto piece = make_shared<SimpleStringPiece>("OK");
  ctx.Reply(piece);
  ctx.Close();
}

void Connection::Select(Ctx& ctx, vector<string> req) {
  if (req.size() != 2) {
    auto piece = make_shared<ErrorPiece>(
        "wrong number of arguments for 'select' command");
    ctx.Reply(piece);
    return;
  }

  int64_t index = 0;
  if (req[1] != "0") {
    index = atoll(req[1].c_str());
    if (index == 0) {
      auto piece =
          make_shared<ErrorPiece>("value is not an integer or out of range");
      ctx.Reply(piece);
      return;
    }
  }

  int ret = ctx.SelectDb(index);
  if (ret != 0) {
    auto piece = make_shared<SimpleStringPiece>("OK");
    ctx.Reply(piece);
    return;
  }
  auto piece = make_shared<ErrorPiece>("DB index is out of range");
  ctx.Reply(piece);
}

}  // namespace mydss::cmd
