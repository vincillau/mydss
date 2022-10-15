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

#include <cmd/string.hpp>

using mydss::module::ArrayPiece;
using mydss::module::BulkStringPiece;
using mydss::module::Ctx;
using mydss::module::ErrorPiece;
using mydss::module::IntegerPiece;
using mydss::module::NullPiece;
using mydss::module::SimpleStringPiece;
using mydss::module::encoding::kInt;
using mydss::module::encoding::kRaw;
using mydss::module::type::kString;
using std::dynamic_pointer_cast;
using std::make_shared;
using std::string;
using std::to_string;
using std::vector;

namespace mydss::cmd {

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

static void StringIncrBy(Ctx& ctx, const string& key, int64_t i64) {
  auto obj = ctx.GetObject(key);
  if (obj == nullptr) {
    auto new_obj = make_shared<String>();
    new_obj->SetI64(i64);
    ctx.SetObject(key, new_obj);
    auto piece = make_shared<IntegerPiece>(i64);
    ctx.Reply(piece);
    return;
  }

  if (obj->type() != kString) {
    auto piece = make_shared<ErrorPiece>(
        "WRONGTYPE Operation against a key holding the wrong kind of value");
    ctx.Reply(piece);
    return;
  }

  auto str = dynamic_pointer_cast<String>(obj);
  if (str->encoding() == kInt) {
    int64_t old_i64 = str->I64();

    if (I64AddOverflow(old_i64, i64)) {
      auto piece =
          make_shared<ErrorPiece>("value is not an integer or out of range");
      ctx.Reply(piece);
      return;
    }

    int64_t new_i64 = old_i64 + i64;
    str->SetI64(new_i64);
    auto piece = make_shared<IntegerPiece>(new_i64);
    ctx.Reply(piece);
    return;
  }
  if (str->encoding() == kRaw) {
    auto piece =
        make_shared<ErrorPiece>("value is not an integer or out of range");
    ctx.Reply(piece);
    return;
  }
  assert(false);
}

void String::SetValue(std::string value) {
  if (value == "0") {
    encoding_ = kInt;
    value_ = 0;
  }

  int64_t i64 = atoll(value.c_str());
  if (i64 != 0) {
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

void String::Append(Ctx& ctx, vector<string> req) {
  if (req.size() != 3) {
    auto piece = make_shared<ErrorPiece>(
        "wrong number of arguments for 'append' command");
    ctx.Reply(piece);
    return;
  }

  const auto& key = req[1];
  const auto& value = req[2];

  auto obj = ctx.GetObject(key);
  if (obj == nullptr) {
    auto new_obj = make_shared<String>(value);
    ctx.SetObject(key, new_obj);
    auto piece = make_shared<IntegerPiece>(value.size());
    ctx.Reply(piece);
    return;
  }

  if (obj->type() != kString) {
    auto piece = make_shared<ErrorPiece>(
        "WRONGTYPE Operation against a key holding the wrong kind of value");
    ctx.Reply(piece);
    return;
  }

  auto str = dynamic_pointer_cast<String>(obj);
  if (str->encoding_ == kInt) {
    auto new_value = to_string(str->I64()) + value;
    str->SetValue(std::move(new_value));
    auto piece = make_shared<IntegerPiece>(new_value.size());
    ctx.Reply(piece);
    return;
  }
  if (str->encoding_ == kRaw) {
    auto new_value = str->Str() + value;
    str->SetValue(std::move(new_value));
    auto piece = make_shared<IntegerPiece>(new_value.size());
    ctx.Reply(piece);
    return;
  }
  assert(false);
}

void String::Decr(Ctx& ctx, vector<string> req) {
  if (req.size() != 2) {
    auto piece =
        make_shared<ErrorPiece>("wrong number of arguments for 'decr' command");
    ctx.Reply(piece);
    return;
  }

  const auto& key = req[1];
  StringIncrBy(ctx, key, -1);
}

void String::DecrBy(Ctx& ctx, vector<string> req) {
  if (req.size() != 3) {
    auto piece = make_shared<ErrorPiece>(
        "wrong number of arguments for 'decrby' command");
    return;
  }

  const auto& key = req[1];
  const auto& value = req[2];

  int64_t i64 = 0;
  if (value != "0") {
    i64 = atoll(value.c_str());

    if (i64 == 0) {
      auto piece =
          make_shared<ErrorPiece>("value is not an integer or out of range");
      ctx.Reply(piece);
      return;
    }
  }

  if (i64 == INT64_MIN) {
    auto piece =
        make_shared<ErrorPiece>("value is not an integer or out of range");
    ctx.Reply(piece);
    return;
  }

  StringIncrBy(ctx, key, -i64);
}

void String::Get(Ctx& ctx, vector<string> req) {
  if (req.size() != 2) {
    auto piece =
        make_shared<ErrorPiece>("wrong number of arguments for 'get' command");
    ctx.Reply(piece);
    return;
  }

  const string& key = req[1];
  auto obj = ctx.GetObject(key);
  if (obj == nullptr) {
    auto piece = make_shared<NullPiece>();
    ctx.Reply(piece);
    return;
  }

  if (obj->type() != kString) {
    auto piece = make_shared<ErrorPiece>(
        "WRONGTYPE Operation against a key holding the wrong kind of value");
    ctx.Reply(piece);
    return;
  }

  auto str = dynamic_pointer_cast<String>(obj);
  if (str->encoding_ == kInt) {
    auto i64_str = to_string(str->I64());
    auto piece = make_shared<BulkStringPiece>(std::move(i64_str));
    ctx.Reply(piece);
    return;
  }
  if (str->encoding_ == kRaw) {
    auto piece = make_shared<BulkStringPiece>(str->Str());
    ctx.Reply(piece);
    return;
  }
  assert(false);
}

void String::GetDel(Ctx& ctx, vector<string> req) {
  if (req.size() != 2) {
    auto piece = make_shared<ErrorPiece>(
        "wrong number of arguments for 'getdel' command");
  }

  const string& key = req[1];
  auto obj = ctx.GetObject(key);
  if (obj == nullptr) {
    auto piece = make_shared<NullPiece>();
    ctx.Reply(piece);
    return;
  }

  if (obj->type() != kString) {
    auto piece = make_shared<ErrorPiece>(
        "WRONGTYPE Operation against a key holding the wrong kind of value");
    ctx.Reply(piece);
    return;
  }

  ctx.DeleteObject(key);
  auto str = dynamic_pointer_cast<String>(obj);
  if (str->encoding_ == kInt) {
    auto i64_str = to_string(str->I64());
    auto piece = make_shared<BulkStringPiece>(std::move(i64_str));
    ctx.Reply(piece);
    return;
  }
  if (str->encoding_ == kRaw) {
    auto piece = make_shared<BulkStringPiece>(str->Str());
    ctx.Reply(piece);
    return;
  }
  assert(false);
}

void String::GetRange(Ctx& ctx, vector<string> req) {
  if (req.size() != 4) {
    auto piece = make_shared<ErrorPiece>(
        "wrong number of arguments for 'getrange' command");
    ctx.Reply(piece);
    return;
  }

  int64_t start = 0;
  if (req[2] != "0") {
    start = atoll(req[2].c_str());
    if (start == 0) {
      auto piece =
          make_shared<ErrorPiece>("value is not an integer or out of range");
      ctx.Reply(piece);
      return;
    }
  }

  int64_t end = 0;
  if (req[3] != "0") {
    end = atoll(req[3].c_str());
    if (end == 0) {
      auto piece =
          make_shared<ErrorPiece>("value is not an integer or out of range");
      ctx.Reply(piece);
      return;
    }
  }

  const string& key = req[1];
  auto obj = ctx.GetObject(key);
  if (obj == nullptr) {
    auto piece = make_shared<BulkStringPiece>();
    ctx.Reply(piece);
    return;
  }

  if (obj->type() != kString) {
    auto piece = make_shared<ErrorPiece>(
        "WRONGTYPE Operation against a key holding the wrong kind of value");
    ctx.Reply(piece);
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
    auto piece = make_shared<BulkStringPiece>();
    ctx.Reply(piece);
    return;
  }

  if (start > value.size() - 1) {
    start = value.size() - 1;
  }
  if (end > value.size() - 1) {
    end = value.size() - 1;
  }

  auto substr = value.substr(start, end - start + 1);
  auto piece = make_shared<BulkStringPiece>(std::move(substr));
  ctx.Reply(piece);
}

void String::Incr(Ctx& ctx, vector<string> req) {
  if (req.size() != 2) {
    auto piece =
        make_shared<ErrorPiece>("wrong number of arguments for 'incr' command");
    ctx.Reply(piece);
    return;
  }

  const auto& key = req[1];
  StringIncrBy(ctx, key, 1);
}

void String::IncrBy(Ctx& ctx, vector<string> req) {
  if (req.size() != 3) {
    auto piece = make_shared<ErrorPiece>(
        "wrong number of arguments for 'incrby' command");
    ctx.Reply(piece);
    return;
  }

  const auto& key = req[1];
  const auto& value = req[2];

  int64_t i64 = 0;
  if (value != "0") {
    i64 = atoll(value.c_str());
    if (i64 == 0) {
      auto piece =
          make_shared<ErrorPiece>("value is not an integer or out of range");
      return;
    }
  }

  StringIncrBy(ctx, key, i64);
}

void String::MGet(Ctx& ctx, vector<string> req) {
  if (req.size() == 1) {
    auto piece =
        make_shared<ErrorPiece>("wrong number of arguments for 'mget' command");
  }

  auto array = make_shared<ArrayPiece>(req.size() - 1);
  ctx.Reply(array);

  for (size_t i = 1; i < req.size(); i++) {
    const string& key = req[i];
    auto obj = ctx.GetObject(key);
    if (obj == nullptr) {
      auto piece = make_shared<NullPiece>();
      ctx.Reply(piece);
      continue;
    }

    if (obj->type() != kString) {
      auto piece = make_shared<NullPiece>();
      ctx.Reply(piece);
      continue;
    }

    auto str = dynamic_pointer_cast<String>(obj);
    if (str->encoding_ == kInt) {
      auto i64_str = to_string(str->I64());
      auto piece = make_shared<BulkStringPiece>(std::move(i64_str));
      ctx.Reply(piece);
      continue;
    }
    if (str->encoding_ == kRaw) {
      auto piece = make_shared<BulkStringPiece>(str->Str());
      ctx.Reply(piece);
      continue;
    }
    assert(false);
  }
}

void String::MSet(Ctx& ctx, vector<string> req) {
  if (req.size() == 1 || req.size() % 2 == 0) {
    auto piece =
        make_shared<ErrorPiece>("wrong number of arguments for 'mset' command");
    ctx.Reply(piece);
    return;
  }

  for (size_t i = 1; i < req.size(); i += 2) {
    const auto& key = req[i];
    const auto& value = req[i + 1];

    auto obj = make_shared<String>(value);
    ctx.SetObject(key, obj);
  }

  auto piece = make_shared<SimpleStringPiece>("OK");
  ctx.Reply(piece);
}

void String::MSetNx(Ctx& ctx, vector<string> req) {
  if (req.size() == 1 || req.size() % 2 == 0) {
    auto piece = make_shared<ErrorPiece>(
        "wrong number of arguments for 'msetnx' command");
    ctx.Reply(piece);
    return;
  }

  int64_t ret = 1;
  for (size_t i = 1; i < req.size(); i += 2) {
    const auto& key = req[i];
    const auto& value = req[i + 1];

    if (ctx.GetObject(key) == nullptr) {
      auto obj = make_shared<String>(value);
      ctx.SetObject(key, obj);
      continue;
    }
    ret = 0;
  }

  auto piece = make_shared<IntegerPiece>(ret);
  ctx.Reply(piece);
}

void String::Set(Ctx& ctx, vector<string> req) {
  if (req.size() != 3) {
    auto piece =
        make_shared<ErrorPiece>("wrong number of arguments for 'set' command");
    ctx.Reply(piece);
    return;
  }

  const auto& key = req[1];
  const auto& value = req[2];

  auto obj = make_shared<String>(value);
  ctx.SetObject(key, obj);
  auto piece = make_shared<SimpleStringPiece>("OK");
  ctx.Reply(piece);
}

void String::StrLen(Ctx& ctx, vector<string> req) {
  if (req.size() != 2) {
    auto piece = make_shared<ErrorPiece>(
        "wrong number of arguments for 'strlen' command");
    ctx.Reply(piece);
    return;
  }

  const string& key = req[1];
  auto obj = ctx.GetObject(key);
  if (obj == nullptr) {
    auto piece = make_shared<IntegerPiece>(0);
    ctx.Reply(piece);
    return;
  }

  if (obj->type() != kString) {
    auto piece = make_shared<ErrorPiece>(
        "WRONGTYPE Operation against a key holding the wrong kind of value");
    ctx.Reply(piece);
    return;
  }

  auto str = dynamic_pointer_cast<String>(obj);
  if (str->encoding_ == kInt) {
    auto i64_str = to_string(str->I64());
    auto piece = make_shared<IntegerPiece>(i64_str.size());
    ctx.Reply(piece);
    return;
  }
  if (str->encoding_ == kRaw) {
    auto piece = make_shared<IntegerPiece>(str->Str().size());
    ctx.Reply(piece);
    return;
  }
  assert(false);
}

}  // namespace mydss::cmd
