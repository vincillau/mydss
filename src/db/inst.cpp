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

#include <cmd/string.hpp>
#include <db/inst.hpp>
#include <util/str.hpp>

using fmt::format;
using mydss::cmd::String;
using mydss::proto::ErrorPiece;
using mydss::proto::Piece;
using mydss::proto::Req;
using mydss::util::StrLower;
using std::make_shared;
using std::shared_ptr;
using std::string;

namespace mydss::db {

shared_ptr<Inst> Inst::inst_;

void Inst::Init(int db_num) {
  assert(db_num > 0);
  inst_ = shared_ptr<Inst>(new Inst(db_num));

  // String
  inst_->RegisterCmd("GET", String::Get);
  inst_->RegisterCmd("SET", String::Set);
}

shared_ptr<Object> Inst::GetObject(const string& key, int db) {
  if (db == -1) {
    db = cur_db_;
  }
  assert(db >= 0 && db < dbs_.size());

  // 检查是否存在
  auto& objs = dbs_[db].objs();
  auto it = objs.find(key);
  if (it == objs.end()) {
    return nullptr;
  }

  // 检查是否过期
  auto& obj = it->second;
  if (obj->Pttl() == 0) {
    objs.erase(it);
    return nullptr;
  }

  return obj;
}

void Inst::SetObject(string key, shared_ptr<Object> obj, int db) {
  if (db == -1) {
    db = cur_db_;
  }
  assert(db >= 0 && db < dbs_.size());

  if (db == -1) {
    db = cur_db_;
  }

  dbs_[db].objs()[std::move(key)] = obj;
}

void Inst::RegisterCmd(string name, Cmd cmd) {
  StrLower(name);
  if (cmds_.find(name) != cmds_.end()) {
    SPDLOG_ERROR("cannot register command '{}', conflicting names", name);
    return;
  }
  cmds_[std::move(name)] = std::move(cmd);
}

void Inst::Handle(const Req& req, shared_ptr<Piece>& resp) {
  if (req.pieces().empty()) {
    resp = make_shared<ErrorPiece>("empty commmand");
    return;
  }

  string cmd_name = req.pieces().front();
  StrLower(cmd_name);

  auto it = cmds_.find(cmd_name);
  if (it == cmds_.end()) {
    string err_str = format("unknown command '{}', with args beginning with:",
                            req.pieces().front());
    for (size_t i = 1; i < req.pieces().size(); i++) {
      err_str += format(" '{}'", req.pieces()[i]);
    }
    resp = make_shared<ErrorPiece>(std::move(err_str));
    return;
  }

  const auto& cmd = it->second;
  cmd(req, resp);
}

}  // namespace mydss::db
