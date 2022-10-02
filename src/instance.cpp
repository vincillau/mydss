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

#include <cassert>
#include <command/generic.hpp>
#include <command/string.hpp>
#include <instance.hpp>
#include <util.hpp>

using fmt::format;
using std::make_shared;
using std::shared_ptr;

namespace mydss {

namespace {

static void HandleUnknownCommand(const Req& req,
                                 std::shared_ptr<Piece>& result) {
  assert(!req.pieces().empty());

  auto it = req.pieces().cbegin();
  auto msg =
      format("unknown command '{}', with args beginning with:", it->value());
  while (++it != req.pieces().cend()) {
    msg += format(" '{}'", it->value());
  }
  result = make_shared<ErrorPiece>(std::move(msg));
}

}  // namespace

shared_ptr<Instance> Instance::inst_;

shared_ptr<Instance> Instance::GetInstance() {
  if (!inst_) {
    inst_ = make_shared<Instance>();
  }
  return inst_;
}

void Instance::RegisterCommand(std::string command_name, Command command) {
  StrToUpper(command_name);
  auto it = commands_.find(command_name);
  if (it != commands_.end()) {
    SPDLOG_ERROR("conflicting command '{}'", command_name);
    commands_.erase(it);
    return;
  }
  SPDLOG_DEBUG("register command '{}'", command_name);
  commands_[command_name] = command;
}

void Instance::Handle(const Req& req, std::shared_ptr<Piece>& result) {
  if (req.pieces().empty()) {
    result = make_shared<ErrorPiece>("empty command");
  }

  auto command_name = req.pieces()[0].value();
  StrToUpper(command_name);
  auto command = commands_.find(command_name);
  if (command == commands_.end()) {
    HandleUnknownCommand(req, result);
    return;
  }

  Status status = (command->second)(*this, req, result);
  if (status.error()) {
    SPDLOG_ERROR("Handle req error: `{}`", status);
  }
}

void Instance::InitModules() {
  // Generic
  RegisterCommand("DEL", generic::Del);
  RegisterCommand("EXISTS", generic::Exists);
  RegisterCommand("KEYS", generic::Keys);
  RegisterCommand("OBJECT", generic::Object);
  RegisterCommand("RENAME", generic::Rename);
  RegisterCommand("RENAMENX", generic::RenameNx);
  RegisterCommand("TYPE", generic::Type);

  // String
  RegisterCommand("GET", string::Get);
  RegisterCommand("SET", string::Set);
}

}  // namespace mydss
