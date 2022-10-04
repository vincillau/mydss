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

#include <sys/time.h>

#include <command/generic.hpp>
#include <regex>
#include <util.hpp>

using fmt::format;
using std::make_shared;
using std::regex;
using std::regex_match;
using std::shared_ptr;
using std::string;

namespace mydss::generic {

Status Del(Instance& inst, const Req& req, shared_ptr<Piece>& piece) {
  if (req.pieces().size() == 1) {
    piece =
        make_shared<ErrorPiece>("wrong number of arguments for 'del' command");
    return Status::Ok();
  }

  auto it = req.pieces().cbegin();
  int64_t ndel = 0;
  while (++it != req.pieces().cend()) {
    auto& db_map = inst.GetCurDb().map();
    ndel += db_map.erase(it->value());
  }

  piece = make_shared<IntegerPiece>(ndel);
  return Status::Ok();
}

Status Exists(Instance& inst, const Req& req, shared_ptr<Piece>& piece) {
  if (req.pieces().size() == 1) {
    piece = make_shared<ErrorPiece>(
        "wrong number of arguments for 'exists' command");
    return Status::Ok();
  }

  auto it = req.pieces().cbegin();
  int64_t nexists = 0;
  while (++it != req.pieces().cend()) {
    const auto& db_map = inst.GetCurDb().map();
    if (db_map.find(it->value()) != db_map.end()) {
      nexists++;
    }
  }

  piece = make_shared<IntegerPiece>(nexists);
  return Status::Ok();
}

Status Expire(Instance& inst, const Req& req, shared_ptr<Piece>& piece) {
  if (req.pieces().size() < 3) {
    piece = make_shared<ErrorPiece>(
        "wrong number of arguments for 'expire' command");
    return Status::Ok();
  }
  const string& key = req.pieces()[1].value();
  const string& seconds = req.pieces()[2].value();
  int64_t sec = 0;
  bool is_num = StrToI64(seconds, &sec);
  if (!is_num) {
    piece = make_shared<ErrorPiece>("value is not an integer or out of range");
    return Status::Ok();
  }

  bool xx = false;
  bool nx = false;
  bool gt = false;
  bool lt = false;
  for (size_t i = 3; i < req.pieces().size(); i++) {
    auto opt = req.pieces()[i].value();
    StrToUpper(opt);
    if (opt == "XX") {
      xx = true;
    } else if (opt == "NX") {
      nx = true;
    } else if (opt == "GT") {
      gt = true;
    } else if (opt == "LT") {
      lt = true;
    } else {
      piece = make_shared<ErrorPiece>(
          format("Unsupported option {}", req.pieces()[i].value()));
      return Status::Ok();
    }
  }

  if (nx && (xx || gt || lt)) {
    piece = make_shared<ErrorPiece>(
        "NX and XX, GT or LT options at the same time are not compatible");
    return Status::Ok();
  }
  if (gt && lt) {
    piece = make_shared<ErrorPiece>(
        "GT and LT options at the same time are not compatible");
    return Status::Ok();
  }

  auto& db_map = inst.GetCurDb().map();
  auto it = db_map.find(key);

  if (it == db_map.end()) {
    piece = make_shared<IntegerPiece>(0);
    return Status::Ok();
  }

  if (nx) {
    if (it->second.HasTtl()) {
      piece = make_shared<IntegerPiece>(0);
    } else {
      it->second.set_ttl(sec);
      piece = make_shared<IntegerPiece>(1);
    }
    return Status::Ok();
  }

  if (gt) {
    if (it->second.HasTtl()) {
      double ttl = it->second.ttl();
      if (sec > ttl) {
        it->second.set_ttl(sec);
        piece = make_shared<IntegerPiece>(1);
      } else {
        piece = make_shared<IntegerPiece>(0);
      }
    } else {
      piece = make_shared<IntegerPiece>(0);
    }
    return Status::Ok();
  }

  if (lt) {
    if (it->second.HasTtl()) {
      double ttl = it->second.ttl();
      if (sec < ttl) {
        it->second.set_ttl(sec);
        piece = make_shared<IntegerPiece>(1);
      } else {
        piece = make_shared<IntegerPiece>(0);
      }
    } else {
      piece = make_shared<IntegerPiece>(0);
    }
    return Status::Ok();
  }

  if (xx) {
    if (it->second.HasTtl()) {
      it->second.set_ttl(sec);
      piece = make_shared<IntegerPiece>(1);
    } else {
      piece = make_shared<IntegerPiece>(0);
    }
    return Status::Ok();
  }

  it->second.set_ttl(sec);
  piece = make_shared<IntegerPiece>(1);
  return Status::Ok();
}

Status ExpireAt(Instance& inst, const Req& req, shared_ptr<Piece>& piece);

Status Keys(Instance& inst, const Req& req, shared_ptr<Piece>& piece) {
  if (req.pieces().size() != 2) {
    piece =
        make_shared<ErrorPiece>("wrong number of arguments for 'keys' command");
    return Status::Ok();
  }

  auto array = make_shared<ArrayPiece>();
  // 暂时使用 C++11 的正则表达式进行匹配
  regex re(req.pieces()[1].value());
  for (const auto [key, _] : inst.GetCurDb().map()) {
    if (regex_match(key, re)) {
      array->pieces().emplace_back(make_shared<BulkStringPiece>(key));
    }
  }

  piece = array;
  return Status::Ok();
}

Status Move(Instance& inst, const Req& req, shared_ptr<Piece>& piece);

Status Object(Instance& inst, const Req& req, shared_ptr<Piece>& piece) {
  if (req.pieces().size() == 1) {
    piece = make_shared<ErrorPiece>(
        "wrong number of arguments for 'object' command");
    return Status::Ok();
  }
  string sub_command = req.pieces()[1].value();
  StrToUpper(sub_command);

  if (sub_command == "ENCODING") {
    return ObjectEncoding(inst, req, piece);
  } else if (sub_command == "IDLETIME") {
    return ObjectIdleTime(inst, req, piece);
  } else if (sub_command == "REFCOUNT") {
    return ObjectRefCount(inst, req, piece);
  }

  piece = make_shared<ErrorPiece>(
      format("ERROR unknown subcommand '{}'. Try OBJECT HELP.",
             req.pieces()[1].value()));
  return Status::Ok();
}

Status ObjectEncoding(Instance& inst, const Req& req,
                      shared_ptr<Piece>& piece) {
  if (req.pieces().size() != 3) {
    piece = make_shared<ErrorPiece>(
        "wrong number of arguments for 'object|encoding' command");
    return Status::Ok();
  }
  const string& key = req.pieces()[2].value();

  const auto& db_map = inst.GetCurDb().map();
  auto it = db_map.find(key);
  if (it == db_map.end()) {
    piece = make_shared<BulkStringPiece>();
  } else {
    piece =
        make_shared<BulkStringPiece>(ObjectEncodingStr(it->second.encoding()));
  }
  return Status::Ok();
}

Status ObjectIdleTime(Instance& inst, const Req& req,
                      shared_ptr<Piece>& piece) {
  if (req.pieces().size() != 3) {
    piece = make_shared<ErrorPiece>(
        "wrong number of arguments for 'object|idletime' command");
    return Status::Ok();
  }
  const string& key = req.pieces()[2].value();

  auto& db_map = inst.GetCurDb().map();
  auto it = db_map.find(key);
  if (it == db_map.end()) {
    piece = make_shared<BulkStringPiece>();
  } else {
    time_t now = time(nullptr);
    piece = make_shared<IntegerPiece>(now - it->second.access_time());
  }
  return Status::Ok();
}

Status ObjectRefCount(Instance& inst, const Req& req,
                      shared_ptr<Piece>& piece) {
  if (req.pieces().size() != 3) {
    piece = make_shared<ErrorPiece>(
        "wrong number of arguments for 'object|refcount' command");
    return Status::Ok();
  }
  const string& key = req.pieces()[2].value();

  const auto& db_map = inst.GetCurDb().map();
  auto it = db_map.find(key);
  if (it == db_map.end()) {
    piece = make_shared<BulkStringPiece>();
  } else {
    piece = make_shared<IntegerPiece>(1);
  }
  return Status::Ok();
}

Status Persist(Instance& inst, const Req& req, shared_ptr<Piece>& piece) {
  if (req.pieces().size() != 2) {
    piece =
        make_shared<ErrorPiece>("wrong number of arguments for 'ttl' command");
    return Status::Ok();
  }
  const string& key = req.pieces()[1].value();

  auto& db_map = inst.GetCurDb().map();
  auto it = db_map.find(key);
  if (it == db_map.end()) {
    piece = make_shared<IntegerPiece>(0);
  } else if (it->second.expire().tv_sec == 0 &&
             it->second.expire().tv_usec == 0) {
    piece = make_shared<IntegerPiece>(0);
  } else {
    it->second.set_expire({0, 0});
    it->second.Touch();
    piece = make_shared<IntegerPiece>(1);
  }

  return Status::Ok();
}

Status PExpire(Instance& inst, const Req& req, shared_ptr<Piece>& piece);

Status PExpireAt(Instance& inst, const Req& req, shared_ptr<Piece>& piece);

Status PTtl(Instance& inst, const Req& req, shared_ptr<Piece>& piece) {
  if (req.pieces().size() != 2) {
    piece =
        make_shared<ErrorPiece>("wrong number of arguments for 'pttl' command");
    return Status::Ok();
  }
  const string& key = req.pieces()[1].value();

  const auto& db_map = inst.GetCurDb().map();
  auto it = db_map.find(key);
  if (it == db_map.end()) {
    piece = make_shared<IntegerPiece>(-2);
  } else if (it->second.expire().tv_sec == 0 &&
             it->second.expire().tv_usec == 0) {
    piece = make_shared<IntegerPiece>(-1);
  } else {
    int64_t pttl = static_cast<int64_t>(it->second.pttl());
    piece = make_shared<IntegerPiece>(pttl);
  }

  return Status::Ok();
}

Status Rename(Instance& inst, const Req& req, shared_ptr<Piece>& piece) {
  if (req.pieces().size() != 3) {
    piece = make_shared<ErrorPiece>(
        "wrong number of arguments for 'rename' command");
    return Status::Ok();
  }
  const string& key = req.pieces()[1].value();
  const string& new_key = req.pieces()[2].value();

  auto& db_map = inst.GetCurDb().map();
  auto it = db_map.find(key);
  if (it == db_map.end()) {
    piece = make_shared<ErrorPiece>("no such key");
  } else {
    db_map.insert({new_key, std::move(it->second)});
    db_map.at(new_key).Touch();
    db_map.erase(key);
    piece = make_shared<SimpleStringPiece>("OK");
  }
  return Status::Ok();
}

Status RenameNx(Instance& inst, const Req& req, shared_ptr<Piece>& piece) {
  if (req.pieces().size() != 3) {
    piece = make_shared<ErrorPiece>(
        "wrong number of arguments for 'renamenx' command");
    return Status::Ok();
  }
  const string& key = req.pieces()[1].value();
  const string& new_key = req.pieces()[2].value();

  auto& db_map = inst.GetCurDb().map();
  auto it = db_map.find(key);
  if (it == db_map.end()) {
    piece = make_shared<ErrorPiece>("no such key");
  } else {
    auto new_key_it = db_map.find(new_key);
    if (new_key_it != db_map.end()) {
      piece = make_shared<IntegerPiece>(0);
      return Status::Ok();
    }
    db_map.insert({new_key, std::move(it->second)});
    db_map.at(new_key).Touch();
    db_map.erase(key);
    piece = make_shared<IntegerPiece>(1);
  }
  return Status::Ok();
}

Status Touch(Instance& inst, const Req& req, shared_ptr<Piece>& piece) {
  if (req.pieces().size() == 1) {
    piece = make_shared<ErrorPiece>(
        "wrong number of arguments for 'touch' command");
    return Status::Ok();
  }

  auto& db_map = inst.GetCurDb().map();
  size_t ntouch = 0;
  for (size_t i = 1; i < req.pieces().size(); i++) {
    const string& key = req.pieces()[i].value();
    auto it = db_map.find(key);
    if (it != db_map.end()) {
      it->second.Touch();
      ntouch++;
    }
  }

  piece = make_shared<IntegerPiece>(ntouch);
  return Status::Ok();
}

Status Ttl(Instance& inst, const Req& req, shared_ptr<Piece>& piece) {
  if (req.pieces().size() != 2) {
    piece =
        make_shared<ErrorPiece>("wrong number of arguments for 'ttl' command");
    return Status::Ok();
  }
  const string& key = req.pieces()[1].value();

  const auto& db_map = inst.GetCurDb().map();
  auto it = db_map.find(key);
  if (it == db_map.end()) {
    piece = make_shared<IntegerPiece>(-2);
  } else if (it->second.expire().tv_sec == 0 &&
             it->second.expire().tv_usec == 0) {
    piece = make_shared<IntegerPiece>(-1);
  } else {
    int64_t ttl = static_cast<int64_t>(it->second.ttl());
    piece = make_shared<IntegerPiece>(ttl);
  }

  return Status::Ok();
}

Status Type(Instance& inst, const Req& req, shared_ptr<Piece>& piece) {
  if (req.pieces().size() != 2) {
    piece =
        make_shared<ErrorPiece>("wrong number of arguments for 'type' command");
    return Status::Ok();
  }
  const string& key = req.pieces()[1].value();
  auto& db_map = inst.GetCurDb().map();
  auto it = db_map.find(key);
  if (it == db_map.end()) {
    piece = make_shared<SimpleStringPiece>("none");
  } else {
    piece = make_shared<SimpleStringPiece>(ObjectTypeStr(it->second.type()));
  }
  return Status::Ok();
}

}  // namespace mydss::generic
