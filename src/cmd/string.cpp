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

using mydss::db::Inst;
using mydss::db::encoding::kInt;
using mydss::db::encoding::kRaw;
using mydss::db::type::kString;
using mydss::proto::BulkStringPiece;
using mydss::proto::ErrorPiece;
using mydss::proto::Piece;
using mydss::proto::Req;
using mydss::proto::SimpleStringPiece;
using mydss::util::StrToI64;
using std::dynamic_pointer_cast;
using std::make_shared;
using std::shared_ptr;
using std::string;
using std::to_string;

namespace mydss::cmd {

void String::SetValue(std::string value) {
  int64_t i64 = 0;
  bool ok = StrToI64(value, &i64);
  if (ok) {
    encoding_ = kInt;
    value_ = i64;
    return;
  }

  encoding_ = kRaw;
  value_ = std::move(value);
}

void String::Get(const Req& req, shared_ptr<Piece>& resp) {
  // 检查参数
  if (req.pieces().size() != 2) {
    resp =
        make_shared<ErrorPiece>("wrong number of arguments for 'get' command");
    return;
  }

  // 检查 key 是否存在
  const string& key = req.pieces()[1];
  auto obj = Inst::GetInst()->GetObject(key);
  if (obj == nullptr) {
    resp = make_shared<BulkStringPiece>();
    return;
  }

  // 检查类型
  if (obj->Type() != kString) {
    resp = make_shared<ErrorPiece>(
        "WRONGTYPE Operation against a key holding the wrong kind of value");
    return;
  }

  auto str = dynamic_pointer_cast<String>(obj);
  if (str->encoding_ == kInt) {
    auto i64_str = to_string(str->I64());
    resp = make_shared<BulkStringPiece>(std::move(i64_str));
    return;
  }
  if (str->encoding_ == kRaw) {
    resp = make_shared<BulkStringPiece>(str->Str());
    return;
  }
}

void String::Set(const Req& req, shared_ptr<Piece>& resp) {
  if (req.pieces().size() != 3) {
    resp =
        make_shared<ErrorPiece>("wrong number of arguments for 'set' command");
    return;
  }

  const auto& key = req.pieces()[1];
  const auto& value = req.pieces()[2];

  auto obj = make_shared<String>(value);
  Inst::GetInst()->SetObject(key, obj);
  resp = make_shared<SimpleStringPiece>("OK");
}

}  // namespace mydss::cmd
