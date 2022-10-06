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

#ifndef MYDSS_INCLUDE_DB_INST_HPP_
#define MYDSS_INCLUDE_DB_INST_HPP_

#include <functional>
#include <proto/piece.hpp>
#include <proto/req.hpp>
#include <vector>

#include "db.hpp"

namespace mydss::db {

// 一个数据库实例
class Inst {
 public:
  using Cmd =
      std::function<void(const proto::Req&, std::shared_ptr<proto::Piece>&)>;

  static void Init(int db_num);
  static std::shared_ptr<Inst> GetInst() { return inst_; }

  [[nodiscard]] std::shared_ptr<Object> GetObject(const std::string& key,
                                                  int db = -1);
  void SetObject(std::string key, std::shared_ptr<Object> obj, int db = -1);

  void RegisterCmd(std::string name, Cmd cmd);
  void Handle(const proto::Req& req, std::shared_ptr<proto::Piece>& resp);

 private:
  Inst(int db_num) : dbs_(db_num) {}

  static std::shared_ptr<Inst> inst_;

 private:
  std::vector<Db> dbs_;
  int cur_db_ = 0;
  std::unordered_map<std::string, Cmd> cmds_;
};

}  // namespace mydss::db

#endif  // MYDSS_INCLUDE_DB_INST_HPP_
