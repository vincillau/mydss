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
using mydss::proto::ArrayPiece;
using mydss::proto::BulkStringPiece;
using mydss::proto::ErrorPiece;
using mydss::proto::IntegerPiece;
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

namespace {

static bool I64AddOverflow(int64_t a, int64_t b) {
  if (a > 0 && b > 0) {
    if (a > INT64_MAX - b) {
      return true;
    }
  }
  if (a < 0 && b < 0) {
    if (a < INT64_MIN - b) {
      return true;
    }
  }
  return false;
}

static void StringIncrBy(const string& key, int64_t i64,
                         shared_ptr<Piece>& resp) {
  // 不存在则创建
  auto inst = Inst::GetInst();
  auto obj = inst->GetObject(key);
  if (obj == nullptr) {
    auto new_obj = make_shared<String>();
    new_obj->SetI64(i64);
    inst->SetObject(key, new_obj);
    resp = make_shared<IntegerPiece>(i64);
    return;
  }

  // 检查类型
  if (obj->Type() != kString) {
    resp = make_shared<ErrorPiece>(
        "WRONGTYPE Operation against a key holding the wrong kind of value");
    return;
  }

  auto str = dynamic_pointer_cast<String>(obj);
  if (str->Encoding() == kInt) {
    int64_t old_i64 = str->I64();

    // 检查是否溢出
    if (I64AddOverflow(old_i64, i64)) {
      resp = make_shared<ErrorPiece>("value is not an integer or out of range");
      return;
    }

    int64_t new_i64 = old_i64 + i64;
    str->SetI64(new_i64);
    resp = make_shared<IntegerPiece>(new_i64);
    return;
  }
  if (str->Encoding() == kRaw) {
    resp = make_shared<ErrorPiece>("value is not an integer or out of range");
    return;
  }
  assert(false);
}

}  // namespace

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

void String::SetI64(int64_t i64) {
  encoding_ = kInt;
  value_ = i64;
}

void String::Append(const Req& req, shared_ptr<Piece>& resp) {
  // 检查参数
  if (req.pieces().size() != 3) {
    resp = make_shared<ErrorPiece>(
        "wrong number of arguments for 'append' command");
    return;
  }

  const auto& key = req.pieces()[1];
  const auto& value = req.pieces()[2];

  // 不存在则创建
  auto inst = Inst::GetInst();
  auto obj = inst->GetObject(key);
  if (obj == nullptr) {
    auto new_obj = make_shared<String>(value);
    inst->SetObject(key, new_obj);
    resp = make_shared<IntegerPiece>(value.size());
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
    auto new_value = to_string(str->I64()) + value;
    resp = make_shared<IntegerPiece>(new_value.size());
    str->SetValue(std::move(new_value));
    return;
  }
  if (str->encoding_ == kRaw) {
    auto new_value = str->Str() + value;
    resp = make_shared<IntegerPiece>(new_value.size());
    str->SetValue(std::move(new_value));
    return;
  }
  assert(false);
}

void String::Decr(const Req& req, shared_ptr<Piece>& resp) {  // 检查参数
  if (req.pieces().size() != 2) {
    resp =
        make_shared<ErrorPiece>("wrong number of arguments for 'decr' command");
    return;
  }

  const auto& key = req.pieces()[1];
  StringIncrBy(key, -1, resp);
}

void String::DecrBy(const Req& req, shared_ptr<Piece>& resp) {
  if (req.pieces().size() != 3) {
    resp = make_shared<ErrorPiece>(
        "wrong number of arguments for 'decrby' command");
    return;
  }

  const auto& key = req.pieces()[1];
  const auto& value = req.pieces()[2];

  int64_t i64 = 0;
  bool ok = StrToI64(value, &i64);
  if (!ok) {
    resp = make_shared<ErrorPiece>("value is not an integer or out of range");
    return;
  }
  if (i64 == INT64_MIN) {
    resp = make_shared<ErrorPiece>("value is not an integer or out of range");
    return;
  }

  StringIncrBy(key, -i64, resp);
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
  assert(false);
}

void String::GetDel(const Req& req, shared_ptr<Piece>& resp) {
  // 检查参数
  if (req.pieces().size() != 2) {
    resp = make_shared<ErrorPiece>(
        "wrong number of arguments for 'getdel' command");
    return;
  }

  // 检查 key 是否存在
  const string& key = req.pieces()[1];
  auto inst = Inst::GetInst();
  auto obj = inst->GetObject(key);
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

  inst->DeleteObject(key);
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
  assert(false);
}

void String::GetEx(const Req& req, shared_ptr<Piece>& resp) {}

void String::GetRange(const Req& req, shared_ptr<Piece>& resp) {  // 检查参数
  if (req.pieces().size() != 4) {
    resp = make_shared<ErrorPiece>(
        "wrong number of arguments for 'getrange' command");
    return;
  }

  int64_t start = 0;
  int64_t end = 0;
  bool ok = StrToI64(req.pieces()[2], &start);
  if (!ok) {
    resp = make_shared<ErrorPiece>("value is not an integer or out of range");
    return;
  }
  ok = StrToI64(req.pieces()[3], &end);
  if (!ok) {
    resp = make_shared<ErrorPiece>("value is not an integer or out of range");
    return;
  }

  // 检查 key 是否存在
  const string& key = req.pieces()[1];
  auto obj = Inst::GetInst()->GetObject(key);
  if (obj == nullptr) {
    resp = make_shared<BulkStringPiece>("");
    return;
  }

  // 检查类型
  if (obj->Type() != kString) {
    resp = make_shared<ErrorPiece>(
        "WRONGTYPE Operation against a key holding the wrong kind of value");
    return;
  }

  auto str = dynamic_pointer_cast<String>(obj);
  string value;
  if (str->encoding_ == kInt) {
    value = to_string(str->I64());

  } else if (str->encoding_ == kRaw) {
    value = str->Str();
  } else {
    assert(false);
  }

  if (start < 0) {
    start += value.size();
  }
  if (end < 0) {
    end += value.size();
  }
  if (start < 0) {
    start = 0;
  }
  if (end < 0) {
    end = 0;
  }
  if (start > end) {
    resp = make_shared<BulkStringPiece>("");
    return;
  }

  if (start > value.size() - 1) {
    start = value.size() - 1;
  }
  if (end > value.size() - 1) {
    end = value.size() - 1;
  }

  auto substr = value.substr(start, end - start + 1);
  resp = make_shared<BulkStringPiece>(std::move(substr));
}

void String::Incr(const Req& req, shared_ptr<Piece>& resp) {
  if (req.pieces().size() != 2) {
    resp =
        make_shared<ErrorPiece>("wrong number of arguments for 'incr' command");
    return;
  }

  const auto& key = req.pieces()[1];
  StringIncrBy(key, 1, resp);
}

void String::IncrBy(const Req& req, shared_ptr<Piece>& resp) {
  if (req.pieces().size() != 3) {
    resp = make_shared<ErrorPiece>(
        "wrong number of arguments for 'incrby' command");
    return;
  }

  const auto& key = req.pieces()[1];
  const auto& value = req.pieces()[2];

  int64_t i64 = 0;
  bool ok = StrToI64(value, &i64);
  if (!ok) {
    resp = make_shared<ErrorPiece>("value is not an integer or out of range");
    return;
  }

  StringIncrBy(key, i64, resp);
}

void String::IncrByFloat(const Req& req, shared_ptr<Piece>& resp) {}

void String::MGet(const Req& req, shared_ptr<Piece>& resp) {
  // 检查参数
  if (req.pieces().size() == 1) {
    resp =
        make_shared<ErrorPiece>("wrong number of arguments for 'mget' command");
    return;
  }

  auto array = make_shared<ArrayPiece>();
  for (size_t i = 1; i < req.pieces().size(); i++) {
    // 检查 key 是否存在
    const string& key = req.pieces()[i];
    auto obj = Inst::GetInst()->GetObject(key);
    if (obj == nullptr) {
      auto piece = make_shared<BulkStringPiece>();
      array->pieces().push_back(piece);
      continue;
    }

    // 检查类型
    if (obj->Type() != kString) {
      auto piece = make_shared<BulkStringPiece>();
      array->pieces().push_back(piece);
      continue;
    }

    auto str = dynamic_pointer_cast<String>(obj);
    if (str->encoding_ == kInt) {
      auto i64_str = to_string(str->I64());
      auto piece = make_shared<BulkStringPiece>(std::move(i64_str));
      array->pieces().push_back(piece);
      continue;
    }
    if (str->encoding_ == kRaw) {
      auto piece = make_shared<BulkStringPiece>(str->Str());
      array->pieces().push_back(piece);
      continue;
    }
    assert(false);
  }

  resp = array;
}

void String::MSet(const Req& req, shared_ptr<Piece>& resp) {
  if (req.pieces().size() == 1 || req.pieces().size() % 2 == 0) {
    resp =
        make_shared<ErrorPiece>("wrong number of arguments for 'mset' command");
    return;
  }

  for (size_t i = 1; i < req.pieces().size(); i += 2) {
    const auto& key = req.pieces()[i];
    const auto& value = req.pieces()[i + 1];

    auto obj = make_shared<String>(value);
    Inst::GetInst()->SetObject(key, obj);
  }

  resp = make_shared<SimpleStringPiece>("OK");
}

void String::MSetNx(const Req& req, shared_ptr<Piece>& resp) {
  if (req.pieces().size() == 1 || req.pieces().size() % 2 == 0) {
    resp = make_shared<ErrorPiece>(
        "wrong number of arguments for 'msetnx' command");
    return;
  }

  int64_t ret = 1;
  for (size_t i = 1; i < req.pieces().size(); i += 2) {
    const auto& key = req.pieces()[i];
    const auto& value = req.pieces()[i + 1];

    auto inst = Inst::GetInst();
    if (inst->GetObject(key) == nullptr) {
      auto obj = make_shared<String>(value);
      inst->SetObject(key, obj);
      continue;
    }
    ret = 0;
  }

  resp = make_shared<IntegerPiece>(ret);
}

void String::MSetEx(const Req& req, shared_ptr<Piece>& resp) {}

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

void String::SetEx(const Req& req, shared_ptr<Piece>& resp) {}
void String::SetNx(const Req& req, shared_ptr<Piece>& resp) {}
void String::SetRange(const Req& req, shared_ptr<Piece>& resp) {}

void String::StrLen(const Req& req, shared_ptr<Piece>& resp) {
  // 检查参数
  if (req.pieces().size() != 2) {
    resp = make_shared<ErrorPiece>(
        "wrong number of arguments for 'strlen' command");
    return;
  }

  // 检查 key 是否存在
  const string& key = req.pieces()[1];
  auto inst = Inst::GetInst();
  auto obj = inst->GetObject(key);
  if (obj == nullptr) {
    resp = make_shared<IntegerPiece>(0);
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
    resp = make_shared<IntegerPiece>(i64_str.size());
    return;
  }
  if (str->encoding_ == kRaw) {
    resp = make_shared<IntegerPiece>(str->Str().size());
    return;
  }
  assert(false);
}

}  // namespace mydss::cmd
