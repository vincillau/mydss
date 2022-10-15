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

#include <fmt/format.h>

#include <cmd/generic.hpp>

using fmt::format;
using mydss::module::BulkStringPiece;
using mydss::module::Ctx;
using mydss::module::ErrorPiece;
using mydss::module::IntegerPiece;
using mydss::module::NullPiece;
using mydss::module::SimpleStringPiece;
using mydss::module::TimeInMsec;
using std::make_shared;
using std::string;
using std::vector;

namespace mydss::cmd {

static void StrLower(string& str) {
  for (char& ch : str) {
    if (ch >= 'A' && ch <= 'Z') {
      ch = ch - 'A' + 'a';
    }
  }
}

static void SetExpire(const string& cmd, Ctx& ctx, vector<string> req) {
  // 检查参数
  if (req.size() < 3) {
    auto piece = make_shared<ErrorPiece>(
        format("wrong number of arguments for '{}' command", cmd));
    ctx.Reply(piece);
    return;
  }

  bool nx = false;
  bool xx = false;
  bool gt = false;
  bool lt = false;

  for (size_t i = 3; i < req.size(); i++) {
    auto opt = req[i];
    StrLower(opt);
    if (opt == "nx") {
      nx = true;
      continue;
    }
    if (opt == "xx") {
      xx = true;
      continue;
    }
    if (opt == "gt") {
      gt = true;
      continue;
    }
    if (opt == "lt") {
      lt = true;
      continue;
    }
    auto piece =
        make_shared<ErrorPiece>(format("Unsupported option {}", req[i]));
    ctx.Reply(piece);
    return;
  }

  if (nx && (xx || gt || lt)) {
    auto piece = make_shared<ErrorPiece>(
        "NX and XX, GT or LT options at the same time are not compatible");
    ctx.Reply(piece);
    return;
  }

  if (gt && lt) {
    auto piece = make_shared<ErrorPiece>(
        "GT and LT options at the same time are not compatible");
    ctx.Reply(piece);
    return;
  }

  const auto& key = req[1];
  const auto& time_str = req[2];
  int64_t time = 0;
  if (time_str != "0") {
    time = atoll(time_str.c_str());
    if (time == 0) {
      auto piece =
          make_shared<ErrorPiece>("value is not an integer or out of range");
      ctx.Reply(piece);
      return;
    }
  }

  int64_t new_pttl = 0;
  if (cmd == "expire") {
    new_pttl = time * 1000;
  } else if (cmd == "pexpire") {
    new_pttl = time;
  } else if (cmd == "expireat") {
    new_pttl = time * 1000 - TimeInMsec();
  } else if (cmd == "pexpireat") {
    new_pttl = time - TimeInMsec();
  } else {
    assert(false);
  }
  if (new_pttl < 0) {
    new_pttl = 0;
  }

  auto obj = ctx.GetObject(key);
  if (obj == nullptr) {
    auto piece = make_shared<IntegerPiece>(0);
    ctx.Reply(piece);
    return;
  }
  int64_t old_pttl = obj->PTtl();

  if (nx) {
    if (old_pttl > 0) {
      // 有过期时间则不设置
      auto piece = make_shared<IntegerPiece>(0);
      ctx.Reply(piece);
    } else {
      obj->SetPTtl(new_pttl);
      auto piece = make_shared<IntegerPiece>(1);
      ctx.Reply(piece);
    }
    return;
  }

  if (xx) {
    // 有 XX 且没有过期时间
    if (old_pttl == -1) {
      auto piece = make_shared<IntegerPiece>(0);
      ctx.Reply(piece);
    } else {
      // XX 和 GT
      if (gt) {
        if (new_pttl > old_pttl) {
          obj->SetPTtl(new_pttl);
          auto piece = make_shared<IntegerPiece>(1);
          ctx.Reply(piece);

        } else {
          auto piece = make_shared<IntegerPiece>(0);
          ctx.Reply(piece);
        }
      }
      // XX 和 LT
      else if (lt) {
        if (new_pttl < old_pttl) {
          obj->SetPTtl(new_pttl);
          auto piece = make_shared<IntegerPiece>(1);
          ctx.Reply(piece);
        } else {
          auto piece = make_shared<IntegerPiece>(0);
          ctx.Reply(piece);
        }
      }
      // 只有 XX
      else {
        obj->SetPTtl(new_pttl);
        auto piece = make_shared<IntegerPiece>(1);
        ctx.Reply(piece);
      }
    }
    return;
  }

  // 只有 GT
  if (gt) {
    if (new_pttl > old_pttl) {
      obj->SetPTtl(new_pttl);
      auto piece = make_shared<IntegerPiece>(1);
      ctx.Reply(piece);
    } else {
      auto piece = make_shared<IntegerPiece>(0);
      ctx.Reply(piece);
    }
  }
  // 只有 LT
  else if (lt) {
    if (new_pttl < old_pttl) {
      obj->SetPTtl(new_pttl);
      auto piece = make_shared<IntegerPiece>(1);
      ctx.Reply(piece);
    } else {
      auto piece = make_shared<IntegerPiece>(0);
      ctx.Reply(piece);
    }
  }
  // 没有选项
  else {
    obj->SetPTtl(new_pttl);
    auto piece = make_shared<IntegerPiece>(1);
    ctx.Reply(piece);
  }
}

void Generic::Del(Ctx& ctx, vector<string> req) {
  // 检查参数
  if (req.size() == 1) {
    auto piece =
        make_shared<ErrorPiece>("wrong number of arguments for 'del' command");
    ctx.Reply(piece);
    return;
  }

  int64_t count = 0;
  for (size_t i = 1; i < req.size(); i++) {
    const auto& key = req[i];
    count += ctx.DeleteObject(key);
  }

  auto piece = make_shared<IntegerPiece>(count);
  ctx.Reply(piece);
}

void Generic::Exists(Ctx& ctx, vector<string> req) {
  // 检查参数
  if (req.size() == 1) {
    auto piece = make_shared<ErrorPiece>(
        "wrong number of arguments for 'exists' command");
    ctx.Reply(piece);
    return;
  }

  int64_t count = 0;
  for (size_t i = 1; i < req.size(); i++) {
    const auto& key = req[i];
    auto obj = ctx.GetObject(key);
    if (obj != nullptr) {
      count++;
    }
  }

  auto piece = make_shared<IntegerPiece>(count);
  ctx.Reply(piece);
}

void Generic::Expire(Ctx& ctx, vector<string> req) {
  SetExpire("expire", ctx, std::move(req));
}

void Generic::ExpireAt(Ctx& ctx, vector<string> req) {
  SetExpire("expireat", ctx, std::move(req));
}

void Generic::Object(Ctx& ctx, vector<string> req) {
  // 检查参数
  if (req.size() == 1) {
    auto piece = make_shared<ErrorPiece>(
        "wrong number of arguments for 'object' command");
    ctx.Reply(piece);
    return;
  }

  auto sub_cmd_name = req[1];
  StrLower(sub_cmd_name);
  if (sub_cmd_name == "encoding") {
    ObjectEncoding(ctx, std::move(req));
    return;
  } else if (sub_cmd_name == "idletime") {
    ObjectIdleTime(ctx, std::move(req));
    return;
  } else if (sub_cmd_name == "refcount") {
    ObjectRefCount(ctx, std::move(req));
    return;
  }
  auto piece = make_shared<ErrorPiece>(
      format("unknown subcommand '{}'. Try OBJECT HELP.", req[1]));
  ctx.Reply(piece);
}

void Generic::ObjectEncoding(Ctx& ctx, vector<string> req) {
  // 检查参数
  if (req.size() != 3) {
    auto piece = make_shared<ErrorPiece>(
        "wrong number of arguments for 'object|encoding' command");
    ctx.Reply(piece);
    return;
  }

  const auto& key = req[2];
  auto obj = ctx.GetObject(key);
  if (obj == nullptr) {
    auto piece = make_shared<NullPiece>();
    ctx.Reply(piece);
    return;
  }
  auto piece = make_shared<BulkStringPiece>(obj->EncodingStr());
  ctx.Reply(piece);
}

void Generic::ObjectIdleTime(Ctx& ctx, vector<string> req) {
  // 检查参数
  if (req.size() != 3) {
    auto piece = make_shared<ErrorPiece>(
        "wrong number of arguments for 'object|idletime' command");
    ctx.Reply(piece);
    return;
  }

  const auto& key = req[2];
  auto obj = ctx.GetObject(key);
  if (obj == nullptr) {
    auto piece = make_shared<NullPiece>();
    ctx.Reply(piece);
    return;
  }
  auto piece = make_shared<IntegerPiece>(obj->IdleTime() / 1000);
  ctx.Reply(piece);
}

void Generic::ObjectRefCount(Ctx& ctx, vector<string> req) {
  if (req.size() != 3) {
    auto piece = make_shared<ErrorPiece>(
        "wrong number of arguments for 'object|refcount' command");
    ctx.Reply(piece);
    return;
  }

  const auto& key = req[2];
  auto obj = ctx.GetObject(key);
  if (obj == nullptr) {
    auto piece = make_shared<NullPiece>();
    ctx.Reply(piece);
    return;
  }
  auto piece = make_shared<IntegerPiece>(1);
  ctx.Reply(piece);
}

void Generic::Persist(Ctx& ctx, vector<string> req) {
  // 检查参数
  if (req.size() != 2) {
    auto piece = make_shared<ErrorPiece>(
        "wrong number of arguments for 'persist' command");
    ctx.Reply(piece);
    return;
  }

  const auto& key = req[1];
  auto obj = ctx.GetObject(key);
  if (obj == nullptr) {
    auto piece = make_shared<IntegerPiece>(0);
    ctx.Reply(piece);
    return;
  }

  int64_t pttl = obj->PTtl();
  if (pttl == -1) {
    auto piece = make_shared<IntegerPiece>(0);
    ctx.Reply(piece);
    return;
  }

  obj->SetPTtl(-1);
  auto piece = make_shared<IntegerPiece>(1);
  ctx.Reply(piece);
}

void Generic::PExpire(Ctx& ctx, vector<string> req) {
  SetExpire("pexpire", ctx, std::move(req));
}

void Generic::PExpireAt(Ctx& ctx, vector<string> req) {
  SetExpire("pexpireat", ctx, std::move(req));
}

void Generic::PTtl(Ctx& ctx, vector<string> req) {
  // 检查参数
  if (req.size() != 2) {
    auto piece =
        make_shared<ErrorPiece>("wrong number of arguments for 'pttl' command");
    ctx.Reply(piece);
    return;
  }

  const auto& key = req[1];
  auto obj = ctx.GetObject(key);
  if (obj == nullptr) {
    auto piece = make_shared<IntegerPiece>(-2);
    ctx.Reply(piece);
    return;
  }

  int64_t pttl = obj->PTtl();
  if (pttl == -1) {
    auto piece = make_shared<IntegerPiece>(-1);
    ctx.Reply(piece);
  } else {
    auto piece = make_shared<IntegerPiece>(pttl);
    ctx.Reply(piece);
  }
}

void Generic::Rename(Ctx& ctx, vector<string> req) {
  // 检查参数
  if (req.size() != 3) {
    auto piece = make_shared<ErrorPiece>(
        "wrong number of arguments for 'rename' command");
    ctx.Reply(piece);
    return;
  }

  const auto& key = req[1];
  const auto& new_key = req[2];

  auto key_obj = ctx.GetObject(key);
  if (key_obj == nullptr) {
    auto piece = make_shared<ErrorPiece>("no such key");
    ctx.Reply(piece);
    return;
  }

  key_obj->Touch();
  ctx.SetObject(new_key, key_obj);
  ctx.DeleteObject(key);
  auto piece = make_shared<SimpleStringPiece>("OK");
  ctx.Reply(piece);
}

void Generic::RenameNx(Ctx& ctx, vector<string> req) {
  // 检查参数
  if (req.size() != 3) {
    auto piece = make_shared<ErrorPiece>(
        "wrong number of arguments for 'renamenx' command");
    ctx.Reply(piece);
    return;
  }

  const auto& key = req[1];
  const auto& new_key = req[2];

  auto key_obj = ctx.GetObject(key);
  if (key_obj == nullptr) {
    auto piece = make_shared<ErrorPiece>("no such key");
    ctx.Reply(piece);
    return;
  }

  auto new_key_obj = ctx.GetObject(new_key);
  if (new_key_obj != nullptr) {
    auto piece = make_shared<IntegerPiece>(0);
    ctx.Reply(piece);
    return;
  }

  key_obj->Touch();
  ctx.SetObject(new_key, key_obj);
  ctx.DeleteObject(key);
  auto piece = make_shared<IntegerPiece>(1);
  ctx.Reply(piece);
}

void Generic::Touch(Ctx& ctx, vector<string> req) {
  // 检查参数
  if (req.size() == 1) {
    auto piece = make_shared<ErrorPiece>(
        "wrong number of arguments for 'touch' command");
    ctx.Reply(piece);
    return;
  }

  int64_t count = 0;
  for (size_t i = 1; i < req.size(); i++) {
    const auto& key = req[i];
    auto obj = ctx.GetObject(key);
    if (obj != nullptr) {
      obj->Touch();
      count++;
    }
  }

  auto piece = make_shared<IntegerPiece>(count);
  ctx.Reply(piece);
}

void Generic::Ttl(Ctx& ctx, vector<string> req) {
  // 检查参数
  if (req.size() != 2) {
    auto piece =
        make_shared<ErrorPiece>("wrong number of arguments for 'ttl' command");
    ctx.Reply(piece);
    return;
  }

  const auto& key = req[1];
  auto obj = ctx.GetObject(key);
  if (obj == nullptr) {
    auto piece = make_shared<IntegerPiece>(-2);
    ctx.Reply(piece);
    return;
  }

  int64_t pttl = obj->PTtl();
  if (pttl == -1) {
    auto piece = make_shared<IntegerPiece>(-1);
    ctx.Reply(piece);
  } else {
    auto piece = make_shared<IntegerPiece>(pttl / 1000);
    ctx.Reply(piece);
  }
}

void Generic::Type(Ctx& ctx, vector<string> req) {
  // 检查参数
  if (req.size() != 2) {
    auto piece =
        make_shared<ErrorPiece>("wrong number of arguments for 'type' command");
    ctx.Reply(piece);
    return;
  }

  const auto& key = req[1];
  auto obj = ctx.GetObject(key);
  if (obj == nullptr) {
    auto piece = make_shared<SimpleStringPiece>("none");
    ctx.Reply(piece);
    return;
  }
  auto piece = make_shared<SimpleStringPiece>(obj->TypeStr());
  ctx.Reply(piece);
}

}  // namespace mydss::cmd
