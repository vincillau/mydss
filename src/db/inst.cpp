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

#include <cmd/connection.hpp>
#include <cmd/generic.hpp>
#include <cmd/string.hpp>
#include <db/inst.hpp>
#include <util/str.hpp>

using fmt::format;
using mydss::cmd::Connection;
using mydss::cmd::Generic;
using mydss::cmd::String;
using mydss::module::Ctx;
using mydss::module::ErrorPiece;
using mydss::module::Req;
using mydss::util::StrLower;
using std::make_shared;
using std::shared_ptr;
using std::string;

namespace mydss::db {

shared_ptr<Inst> Inst::inst_;

void Inst::Init(int db_num) {
  assert(db_num > 0);
  inst_ = shared_ptr<Inst>(new Inst(db_num));

  // Generic
  inst_->RegisterCmd("DEL", Generic::Del);
  inst_->RegisterCmd("EXISTS", Generic::Exists);
  inst_->RegisterCmd("EXPIRE", Generic::Expire);
  inst_->RegisterCmd("EXPIREAT", Generic::ExpireAt);
  inst_->RegisterCmd("OBJECT", Generic::Object);
  inst_->RegisterCmd("PERSIST", Generic::Persist);
  inst_->RegisterCmd("PEXPIRE", Generic::PExpire);
  inst_->RegisterCmd("PEXPIREAT", Generic::PExpireAt);
  inst_->RegisterCmd("PTTL", Generic::PTtl);
  inst_->RegisterCmd("RENAME", Generic::Rename);
  inst_->RegisterCmd("RENAMENX", Generic::RenameNx);
  inst_->RegisterCmd("TOUCH", Generic::Touch);
  inst_->RegisterCmd("TTL", Generic::Ttl);
  inst_->RegisterCmd("TYPE", Generic::Type);

  // String
  inst_->RegisterCmd("APPEND", String::Append);
  inst_->RegisterCmd("DECR", String::Decr);
  inst_->RegisterCmd("DECRBY", String::DecrBy);
  inst_->RegisterCmd("GET", String::Get);
  inst_->RegisterCmd("GETDEL", String::GetDel);
  inst_->RegisterCmd("GETRANGE", String::GetRange);
  inst_->RegisterCmd("INCR", String::Incr);
  inst_->RegisterCmd("INCRBY", String::IncrBy);
  inst_->RegisterCmd("MGET", String::MGet);
  inst_->RegisterCmd("MSET", String::MSet);
  inst_->RegisterCmd("MSETNX", String::MSetNx);
  inst_->RegisterCmd("SET", String::Set);
  inst_->RegisterCmd("STRLEN", String::StrLen);

  // Connnection Management
  inst_->RegisterCmd("CLIENT", Connection::Client);
  inst_->RegisterCmd("ECHO", Connection::Echo);
  inst_->RegisterCmd("PING", Connection::Ping);
  inst_->RegisterCmd("QUIT", Connection::Quit);
  inst_->RegisterCmd("SELECT", Connection::Select);
}

void Inst::RegisterCmd(string name, Cmd cmd) {
  StrLower(name);
  if (cmds_.find(name) != cmds_.end()) {
    SPDLOG_ERROR("cannot register command '{}', conflicting names", name);
    return;
  }
  cmds_[std::move(name)] = std::move(cmd);
}

void Inst::Handle(Ctx& ctx, Req req) {
  if (req.empty()) {
    auto piece = make_shared<ErrorPiece>("empty commmand");
    ctx.Reply(piece);
    return;
  }

  string cmd_name = req.front();
  StrLower(cmd_name);

  auto it = cmds_.find(cmd_name);
  if (it == cmds_.end()) {
    string err_str =
        format("unknown command '{}', with args beginning with:", req.front());
    for (size_t i = 1; i < req.size(); i++) {
      err_str += format(" '{}'", req[i]);
    }
    auto piece = make_shared<ErrorPiece>(std::move(err_str));
    ctx.Reply(piece);
    return;
  }

  const auto& cmd = it->second;
  cmd(ctx, std::move(req));
}

}  // namespace mydss::db
