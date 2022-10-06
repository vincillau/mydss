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

#include <cmd/generic.hpp>
#include <db/inst.hpp>
#include <util/str.hpp>
#include <util/time.hpp>

using fmt::format;
using mydss::db::Inst;
using mydss::proto::BulkStringPiece;
using mydss::proto::ErrorPiece;
using mydss::proto::IntegerPiece;
using mydss::proto::Piece;
using mydss::proto::Req;
using mydss::proto::SimpleStringPiece;
using mydss::util::StrLower;
using mydss::util::StrToI64;
using mydss::util::TimeInMsec;
using std::make_shared;
using std::shared_ptr;
using std::string;

namespace mydss::cmd {

namespace {

static void SetExpire(const string& cmd, const Req& req,
                      shared_ptr<Piece>& resp) {
  // 检查参数
  if (req.pieces().size() < 3) {
    resp = make_shared<ErrorPiece>(
        format("wrong number of arguments for '{}' command", cmd));
    return;
  }

  bool nx = false;
  bool xx = false;
  bool gt = false;
  bool lt = false;

  for (size_t i = 3; i < req.pieces().size(); i++) {
    auto opt = req.pieces()[i];
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
    resp = make_shared<ErrorPiece>(
        format("Unsupported option {}", req.pieces()[i]));
    return;
  }

  if (nx && (xx || gt || lt)) {
    resp = make_shared<ErrorPiece>(
        "NX and XX, GT or LT options at the same time are not compatible");
    return;
  }

  if (gt && lt) {
    resp = make_shared<ErrorPiece>(
        "GT and LT options at the same time are not compatible");
    return;
  }

  const auto& key = req.pieces()[1];
  const auto& time_str = req.pieces()[2];
  int64_t time = 0;
  bool ok = StrToI64(time_str, &time);
  if (!ok) {
    resp = make_shared<ErrorPiece>("value is not an integer or out of range");
    return;
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

  auto obj = Inst::GetInst()->GetObject(key);
  if (obj == nullptr) {
    resp = make_shared<IntegerPiece>(0);
    return;
  }
  int64_t old_pttl = obj->Pttl();

  if (nx) {
    if (old_pttl > 0) {
      // 有过期时间则不设置
      resp = make_shared<IntegerPiece>(0);
    } else {
      obj->SetPttl(new_pttl);
      resp = make_shared<IntegerPiece>(1);
    }
    return;
  }

  if (xx) {
    // 有 XX 且没有过期时间
    if (old_pttl == -1) {
      resp = make_shared<IntegerPiece>(0);
    } else {
      // XX 和 GT
      if (gt) {
        if (new_pttl > old_pttl) {
          obj->SetPttl(new_pttl);
          resp = make_shared<IntegerPiece>(1);

        } else {
          resp = make_shared<IntegerPiece>(0);
        }
      }
      // XX 和 LT
      else if (lt) {
        if (new_pttl < old_pttl) {
          obj->SetPttl(new_pttl);
          resp = make_shared<IntegerPiece>(1);
        } else {
          resp = make_shared<IntegerPiece>(0);
        }
      }
      // 只有 XX
      else {
        obj->SetPttl(new_pttl);
        resp = make_shared<IntegerPiece>(1);
      }
    }
    return;
  }

  // 只有 GT
  if (gt) {
    if (new_pttl > old_pttl) {
      obj->SetPttl(new_pttl);
      resp = make_shared<IntegerPiece>(1);

    } else {
      resp = make_shared<IntegerPiece>(0);
    }
  }
  // 只有 LT
  else if (lt) {
    if (new_pttl < old_pttl) {
      obj->SetPttl(new_pttl);
      resp = make_shared<IntegerPiece>(1);
    } else {
      resp = make_shared<IntegerPiece>(0);
    }
  }
  // 没有选项
  else {
    obj->SetPttl(new_pttl);
    resp = make_shared<IntegerPiece>(1);
  }
}

}  // namespace

void Generic::Del(const Req& req, shared_ptr<Piece>& resp) {
  // 检查参数
  if (req.pieces().size() == 1) {
    resp =
        make_shared<ErrorPiece>("wrong number of arguments for 'del' command");
    return;
  }

  int64_t count = 0;
  for (size_t i = 1; i < req.pieces().size(); i++) {
    const auto& key = req.pieces()[i];
    count += Inst::GetInst()->DeleteObject(key);
  }

  resp = make_shared<IntegerPiece>(count);
}

void Generic::Exists(const Req& req, shared_ptr<Piece>& resp) {
  // 检查参数
  if (req.pieces().size() == 1) {
    resp = make_shared<ErrorPiece>(
        "wrong number of arguments for 'exists' command");
    return;
  }

  int64_t count = 0;
  for (size_t i = 1; i < req.pieces().size(); i++) {
    const auto& key = req.pieces()[i];
    auto obj = Inst::GetInst()->GetObject(key);
    if (obj != nullptr) {
      count++;
    }
  }

  resp = make_shared<IntegerPiece>(count);
}

void Generic::Expire(const Req& req, shared_ptr<Piece>& resp) {
  SetExpire("expire", req, resp);
}

void Generic::ExpireAt(const Req& req, shared_ptr<Piece>& resp) {
  SetExpire("expireat", req, resp);
}

void Generic::Object(const Req& req, shared_ptr<Piece>& resp) {
  // 检查参数
  if (req.pieces().size() == 1) {
    resp =
        make_shared<ErrorPiece>("wrong number of arguments for 'pttl' command");
    return;
  }

  auto sub_cmd_name = req.pieces()[1];
  StrLower(sub_cmd_name);
  if (sub_cmd_name == "encoding") {
    ObjectEncoding(req, resp);
    return;
  }
  if (sub_cmd_name == "idletime") {
    ObjectIdleTime(req, resp);
    return;
  }
  if (sub_cmd_name == "refcount") {
    ObjectRefCount(req, resp);
    return;
  }
  resp = make_shared<ErrorPiece>(
      format("unknown subcommand '{}'. Try OBJECT HELP.", req.pieces()[1]));
}

void Generic::ObjectEncoding(const Req& req, shared_ptr<Piece>& resp) {
  // 检查参数
  if (req.pieces().size() != 3) {
    resp = make_shared<ErrorPiece>(
        "wrong number of arguments for 'object|encoding' command");
    return;
  }

  const auto& key = req.pieces()[2];
  auto obj = Inst::GetInst()->GetObject(key);
  if (obj == nullptr) {
    resp = make_shared<BulkStringPiece>();
    return;
  }
  resp = make_shared<BulkStringPiece>(obj->EncodingStr());
}

void Generic::ObjectIdleTime(const Req& req,
                             shared_ptr<Piece>& resp) {  // 检查参数
  if (req.pieces().size() != 3) {
    resp = make_shared<ErrorPiece>(
        "wrong number of arguments for 'object|idletime' command");
    return;
  }

  const auto& key = req.pieces()[2];
  auto obj = Inst::GetInst()->GetObject(key);
  if (obj == nullptr) {
    resp = make_shared<BulkStringPiece>();
    return;
  }
  resp = make_shared<IntegerPiece>(obj->IdleTime() / 1000);
}

void Generic::ObjectRefCount(const Req& req, shared_ptr<Piece>& resp) {
  if (req.pieces().size() != 3) {
    resp = make_shared<ErrorPiece>(
        "wrong number of arguments for 'object|refcount' command");
    return;
  }

  const auto& key = req.pieces()[2];
  auto obj = Inst::GetInst()->GetObject(key);
  if (obj == nullptr) {
    resp = make_shared<BulkStringPiece>();
    return;
  }
  resp = make_shared<IntegerPiece>(1);
}

void Generic::Persist(const Req& req, shared_ptr<Piece>& resp) {
  // 检查参数
  if (req.pieces().size() != 2) {
    resp = make_shared<ErrorPiece>(
        "wrong number of arguments for 'persist' command");
    return;
  }

  const auto& key = req.pieces()[1];
  auto obj = Inst::GetInst()->GetObject(key);
  if (obj == nullptr) {
    resp = make_shared<IntegerPiece>(0);
    return;
  }

  int64_t pttl = obj->Pttl();
  if (pttl == -1) {
    resp = make_shared<IntegerPiece>(0);
    return;
  }

  obj->SetPttl(-1);
  resp = make_shared<IntegerPiece>(1);
}

void Generic::PExpire(const Req& req, shared_ptr<Piece>& resp) {
  SetExpire("pexpire", req, resp);
}

void Generic::PExpireAt(const Req& req, shared_ptr<Piece>& resp) {
  SetExpire("pexpireat", req, resp);
}

void Generic::Pttl(const Req& req, shared_ptr<Piece>& resp) {
  // 检查参数
  if (req.pieces().size() != 2) {
    resp =
        make_shared<ErrorPiece>("wrong number of arguments for 'pttl' command");
    return;
  }

  const auto& key = req.pieces()[1];
  auto obj = Inst::GetInst()->GetObject(key);
  if (obj == nullptr) {
    resp = make_shared<IntegerPiece>(-2);
    return;
  }

  int64_t pttl = obj->Pttl();
  if (pttl == -1) {
    resp = make_shared<IntegerPiece>(-1);
  } else {
    resp = make_shared<IntegerPiece>(pttl);
  }
}

void Generic::Rename(const Req& req, shared_ptr<Piece>& resp) {
  // 检查参数
  if (req.pieces().size() != 3) {
    resp = make_shared<ErrorPiece>(
        "wrong number of arguments for 'rename' command");
    return;
  }

  const auto& key = req.pieces()[1];
  const auto& new_key = req.pieces()[2];
  auto inst = Inst::GetInst();

  auto key_obj = inst->GetObject(key);
  if (key_obj == nullptr) {
    resp = make_shared<ErrorPiece>("no such key");
    return;
  }

  key_obj->Touch();
  inst->SetObject(new_key, key_obj);
  inst->DeleteObject(key);
  resp = make_shared<SimpleStringPiece>("OK");
}

void Generic::RenameNX(const Req& req, shared_ptr<Piece>& resp) {
  // 检查参数
  if (req.pieces().size() != 3) {
    resp = make_shared<ErrorPiece>(
        "wrong number of arguments for 'renamenx' command");
    return;
  }

  const auto& key = req.pieces()[1];
  const auto& new_key = req.pieces()[2];
  auto inst = Inst::GetInst();

  auto key_obj = inst->GetObject(key);
  if (key_obj == nullptr) {
    resp = make_shared<ErrorPiece>("no such key");
    return;
  }

  auto new_key_obj = inst->GetObject(new_key);
  if (new_key_obj != nullptr) {
    resp = make_shared<IntegerPiece>(0);
    return;
  }

  key_obj->Touch();
  inst->SetObject(new_key, key_obj);
  inst->DeleteObject(key);
  resp = make_shared<IntegerPiece>(1);
}

void Generic::Touch(const Req& req, shared_ptr<Piece>& resp) {
  // 检查参数
  if (req.pieces().size() == 1) {
    resp = make_shared<ErrorPiece>(
        "wrong number of arguments for 'touch' command");
    return;
  }

  int64_t count = 0;
  for (size_t i = 1; i < req.pieces().size(); i++) {
    const auto& key = req.pieces()[i];
    auto obj = Inst::GetInst()->GetObject(key);
    if (obj != nullptr) {
      obj->Touch();
      count++;
    }
  }

  resp = make_shared<IntegerPiece>(count);
}

void Generic::Ttl(const Req& req, shared_ptr<Piece>& resp) {
  // 检查参数
  if (req.pieces().size() != 2) {
    resp =
        make_shared<ErrorPiece>("wrong number of arguments for 'ttl' command");
    return;
  }

  const auto& key = req.pieces()[1];
  auto obj = Inst::GetInst()->GetObject(key);
  if (obj == nullptr) {
    resp = make_shared<IntegerPiece>(-2);
    return;
  }

  int64_t pttl = obj->Pttl();
  if (pttl == -1) {
    resp = make_shared<IntegerPiece>(-1);
  } else {
    resp = make_shared<IntegerPiece>(pttl / 1000);
  }
}

void Generic::Type(const Req& req, shared_ptr<Piece>& resp) {
  // 检查参数
  if (req.pieces().size() != 2) {
    resp =
        make_shared<ErrorPiece>("wrong number of arguments for 'type' command");
    return;
  }

  const auto& key = req.pieces()[1];
  auto obj = Inst::GetInst()->GetObject(key);
  if (obj == nullptr) {
    resp = make_shared<SimpleStringPiece>("none");
    return;
  }
  resp = make_shared<SimpleStringPiece>(obj->TypeStr());
}

}  // namespace mydss::cmd
