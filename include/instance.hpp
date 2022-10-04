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

#ifndef MYDSS_INCLUDE_INSTANCE_HPP_
#define MYDSS_INCLUDE_INSTANCE_HPP_

#include <spdlog/spdlog.h>

#include <db/db.hpp>
#include <functional>
#include <resp/req.hpp>
#include <status.hpp>
#include <vector>

namespace mydss {

class Instance {
 public:
  using Command = typename std::function<Status(Instance&, const Req&,
                                                std::shared_ptr<Piece>&)>;

  Instance() : dbs_(16), db_index_(0) { InitModules(); }

  [[nodiscard]] const auto& dbs() const { return dbs_; }
  [[nodiscard]] auto& dbs() { return dbs_; }

  [[nodiscard]] const auto& CurDb() const { return dbs_[db_index_]; }
  [[nodiscard]] auto& GetCurDb() { return dbs_[db_index_]; }

  void RegisterCommand(std::string command_name, Command command);
  void Handle(const Req& req, std::shared_ptr<Piece>& result);
  void Timeout();

  static void Init();
  [[nodiscard]] static std::shared_ptr<Instance> GetInstance() {
    return inst_;
  };

 private:
  void InitModules();

  static std::shared_ptr<Instance> inst_;

 private:
  std::unordered_map<std::string, Command> commands_;
  std::vector<Db> dbs_;
  int db_index_;
};

inline void Instance::Timeout() {
  for (Db& db : dbs_) {
    db.Timeout();
  }
}

}  // namespace mydss

#endif  // MYDSS_INCLUDE_INSTANCE_HPP_
