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

#ifndef MYDSS_INCLUDE_DB_OBJECT_HPP_
#define MYDSS_INCLUDE_DB_OBJECT_HPP_

#include <fmt/ostream.h>
#include <spdlog/spdlog.h>
#include <sys/time.h>

#include <cassert>
#include <ctime>
#include <list>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>

namespace mydss {

class Object {
 public:
  using Int = std::int64_t;
  using Raw = std::string;
  using List = typename std::list<std::string>;
  using Hash = typename std::unordered_map<std::string, std::string>;
  using Set = typename std::unordered_set<std::string>;
  using Zset = typename std::set<std::string>;

  enum class Type { kString, kList, kHash, kSet, kZset };
  enum class Encoding {
    kInt,
    kEmbStr,
    kRaw,
    kHT,
    kLinkedList,
    kZipList,
    kIntSet,
    kSkipList
  };

  explicit Object(Type type) { SetType(type); }
  explicit Object(std::string str) { set_str(std::move(str)); }
  explicit Object(List list)
      : data_(std::move(list)),
        type_(Type::kList),
        encoding_(Encoding::kLinkedList) {}
  explicit Object(Hash hash)
      : data_(std::move(hash)), type_(Type::kHash), encoding_(Encoding::kHT) {}
  explicit Object(Set set)
      : data_(std::move(set)), type_(Type::kSet), encoding_(Encoding::kHT) {}
  explicit Object(Zset zset)
      : data_(std::move(zset)), type_(Type::kZset), encoding_(Encoding::kHT) {}

  [[nodiscard]] auto type() const { return type_; }
  [[nodiscard]] auto encoding() const { return encoding_; }

  std::string str() const;
  void set_str(std::string str);

  auto access_time() const { return access_time_; }

  const auto& expire() const { return expire_; }
  void set_expire(const timeval& expire) { expire_ = expire; }

  [[nodiscard]] double ttl() const;
  void set_ttl(double ttl);
  [[nodiscard]] double pttl() const;
  [[nodiscard]] bool HasTtl() const;

  const auto& list() const {
    // 调用者负责检查 Object 的类型
    assert(type_ == Type::kList);
    assert(encoding_ == Encoding::kLinkedList);
    return std::get<List>(data_);
  }
  auto& list() {
    // 调用者负责检查 Object 的类型
    assert(type_ == Type::kList);
    assert(encoding_ == Encoding::kLinkedList);
    return std::get<List>(data_);
  }

  const auto& hash() const {
    // 调用者负责检查 Object 的类型
    assert(type_ == Type::kHash);
    assert(encoding_ == Encoding::kHT);
    return std::get<Hash>(data_);
  }
  auto& hash() {
    // 调用者负责检查 Object 的类型
    assert(type_ == Type::kHash);
    assert(encoding_ == Encoding::kHT);
    return std::get<Hash>(data_);
  }

  const auto& set() const {
    // 调用者负责检查 Object 的类型
    assert(type_ == Type::kSet);
    assert(encoding_ == Encoding::kHT);
    return std::get<Set>(data_);
  }
  auto& set() {
    // 调用者负责检查 Object 的类型
    assert(type_ == Type::kSet);
    assert(encoding_ == Encoding::kHT);
    return std::get<Set>(data_);
  }

  const auto& zset() const {
    // 调用者负责检查 Object 的类型
    assert(type_ == Type::kZset);
    assert(encoding_ == Encoding::kHT);
    return std::get<List>(data_);
  }
  auto& zset() {
    // 调用者负责检查 Object 的类型
    assert(type_ == Type::kZset);
    assert(encoding_ == Encoding::kHT);
    return std::get<Zset>(data_);
  }

  void SetType(Type type);
  void Touch() {
    access_time_ = std::time(nullptr);
    SPDLOG_DEBUG("touch object, access_time={}", access_time_);
  }

 private:
  Type type_;
  Encoding encoding_;
  std::variant<Int, Raw, List, Hash, Set, Zset> data_;
  std::time_t access_time_ = 0;
  // timeval.tv.sec 和 timeval.tv_usec 都为 0 时表示永不过期
  timeval expire_{0, 0};
};

inline std::string ObjectTypeStr(Object::Type type) {
  switch (type) {
    case Object::Type::kString:
      return "string";
    case Object::Type::kList:
      return "list";
    case Object::Type::kHash:
      return "hash";
    case Object::Type::kSet:
      return "set";
    case Object::Type::kZset:
      return "zset";
  }
}

inline std::string ObjectEncodingStr(Object::Encoding encoding) {
  switch (encoding) {
    case Object::Encoding::kInt:
      return "int";
      break;
    case Object::Encoding::kEmbStr:
      return "embstr";
      break;
    case Object::Encoding::kRaw:
      return "raw";
      break;
    case Object::Encoding::kHT:
      return "hashtable";
      break;
    case Object::Encoding::kLinkedList:
      return "linkedlist";
      break;
    case Object::Encoding::kZipList:
      return "ziplist";
      break;
    case Object::Encoding::kIntSet:
      return "intset";
      break;
    case Object::Encoding::kSkipList:
      return "skiplist";
      break;
  }
}

inline std::ostream& operator<<(std::ostream& os, Object::Encoding encoding) {
  os << ObjectEncodingStr(encoding);
  return os;
}

inline double Object::ttl() const {
  timeval now;
  gettimeofday(&now, nullptr);
  double ttl = (expire_.tv_sec - now.tv_sec) +
               (expire_.tv_usec - now.tv_usec) / (1000.0 * 1000.0);
  return ttl;
}

inline void Object::set_ttl(double ttl) {
  std::int64_t sec = static_cast<std::int64_t>(ttl);
  std::int64_t usec = static_cast<std::int64_t>((ttl - sec) * 1000.0 * 1000.0);
  timeval now;
  gettimeofday(&now, nullptr);
  now.tv_sec += sec;
  now.tv_usec += usec;
  expire_ = now;
}

inline double Object::pttl() const {
  timeval now;
  gettimeofday(&now, nullptr);
  double pttl = (expire_.tv_sec - now.tv_sec) * 1000.0 +
                (expire_.tv_usec - now.tv_usec) / 1000.0;
  return pttl;
}

inline bool Object::HasTtl() const {
  return !(expire_.tv_sec == 0 && expire_.tv_usec == 0);
}

}  // namespace mydss

#endif  // MYDSS_INCLUDE_DB_OBJECT_HPP_
