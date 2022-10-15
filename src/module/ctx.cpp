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

#include <db/inst.hpp>
#include <module/ctx.hpp>
#include <module/log.hpp>
#include <server/session.hpp>

using mydss::db::Inst;
using mydss::server::Session;
using std::shared_ptr;
using std::string;

namespace mydss::module {

shared_ptr<Object> Ctx::GetObject(const string& key) {
  auto inst = Inst::GetInst();
  auto& objs = inst->db().objs();
  auto it = objs.find(key);
  if (it == objs.end()) {
    return nullptr;
  }

  const auto& obj = it->second;
  if (obj->PTtl() == 0) {
    objs.erase(it);
    return nullptr;
  }
  return obj;
}

void Ctx::SetObject(const string& key, shared_ptr<Object> obj) {
  auto inst = Inst::GetInst();
  auto& objs = inst->db().objs();
  objs[key] = obj;
}

bool Ctx::DeleteObject(const std::string& key) {
  auto inst = Inst::GetInst();
  auto& objs = inst->db().objs();
  auto it = objs.find(key);
  if (it == objs.end()) {
    return false;
  }

  const auto& obj = it->second;
  if (obj->PTtl() == 0) {
    objs.erase(it);
    return false;
  }
  objs.erase(it);
  return true;
}

void Ctx::Reply(shared_ptr<Piece> piece) {
  auto session = Session::GetSession(session_id_);
  session->Send(piece);
}

int Ctx::SelectDb(int db) {
  auto inst = Inst::GetInst();
  if (inst->Select(db)) {
    return 0;
  }
  return -1;
}

void Ctx::Close() {
  auto session = Session::GetSession(session_id_);
  session->Send(nullptr, true);
}

const string& Ctx::GetClientName() {
  auto session = Session::GetSession(session_id_);
  return session->client().name();
}

const void Ctx::SetClientName(string name) {
  auto session = Session::GetSession(session_id_);
  return session->client().set_name(std::move(name));
}

const int64_t Ctx::GetClientId() { return session_id_; }

}  // namespace mydss::module
