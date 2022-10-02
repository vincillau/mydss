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

#include <command/string.hpp>

using fmt::format;
using std::make_shared;
using std::shared_ptr;

namespace mydss::string {
Status Set(Instance& inst, const Req& req, shared_ptr<Piece>& piece) {
  if (req.pieces().size() != 3) {
    piece = make_shared<ErrorPiece>(format(
        "wrong number of arguments for '{}' command", req.pieces()[0].value()));
    return Status::Ok();
  }
  const auto& key = req.pieces()[1];
  const auto& value = req.pieces()[2];

  auto& db_map = inst.GetCurDb().map();
  auto it = db_map.find(key.value());
  if (it != db_map.end()) {
    it->second.set_str(value.value());
  } else {
    db_map.insert({key.value(), Object(value.value())});
  }

  piece = make_shared<SimpleStringPiece>("OK");
  return Status::Ok();
}

Status Get(Instance& inst, const Req& req, shared_ptr<Piece>& piece) {
  if (req.pieces().size() != 2) {
    piece = make_shared<ErrorPiece>(format(
        "wrong number of arguments for '{}' command", req.pieces()[0].value()));
    return Status::Ok();
  }
  const auto& key = req.pieces()[1];

  const auto& db_map = inst.GetCurDb().map();
  auto it = db_map.find(key.value());
  if (it != db_map.end()) {
    if (it->second.type() != Object::Type::kString) {
      piece = make_shared<ErrorPiece>(
          "WRONGTYPE Operation against a key holding the wrong kind of value");
      return Status::Ok();
    }
    piece = make_shared<BulkStringPiece>(it->second.str());
  } else {
    piece = make_shared<BulkStringPiece>();
  }

  return Status::Ok();
}

}  // namespace mydss::string
