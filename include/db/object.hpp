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

#include <cassert>
#include <list>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>

namespace mydss::db {

class Object {
 public:
  // 对象编码类型
  using I64 = int64_t;
  using Str = std::string;
  using List = typename std::list<std::string>;
  using Hash = typename std::unordered_map<std::string, std::string>;
  using Set = typename std::unordered_set<std::string>;
  using Zset = typename std::set<std::string>;

  enum class Type;
  enum class Encoding;

  // 构建指定类型的对象
  explicit Object(I64 i64);
  explicit Object(Str str);
  explicit Object(List list);
  explicit Object(Hash hash);
  explicit Object(Set set);
  explicit Object(Zset zset);

  // 获得对象的值，操作对象的类型和编码必须一致
  I64 i64() const;
  const Str& str() const;
  const List& list() const;
  const Hash& hash() const;
  const Set& set() const;
  const Zset& zset() const;

  // 设置对象的值，操作对象的类型和编码必须一致
  void set_i64(I64 i64);
  void set_str(Str str);
  void set_list(List list);
  void set_hash(Hash hash);
  void set_set(Set set);
  void set_zset(Zset zset);

  // 获取对象的类型
  [[nodiscard]] auto type() const { return type_; }
  // 设置对象的类型，将自动设置编码，原数据将会被清空
  void set_type(Type type);
  // 返回对象类型的字符串表示形式
  std::string TypeStr() const;

  // 获取对象的编码
  [[nodiscard]] auto encoding() const { return encoding_; }
  void set_encoding(Encoding encoding);
  // 返回对象编码的字符串表示形式
  std::string EncodingStr() const;

  // 获取对象的最后访问时间
  [[nodiscard]] auto access_time() const { return access_time_; }
  // 更新对象的最后访问时间
  void Touch();

  // 获取对象的绝对过期时间
  [[nodiscard]] auto expire_time() const { return expire_time_; }
  // 相对过期时间 0 表示已经过期，为 -1 表示永不过期
  // 获取对象的相对过期时间，单位毫秒
  [[nodiscard]] int64_t pttl() const;
  // 设置对象的相对过期时间，单位毫秒，允许为 0 或 -1
  void set_pttl(int64_t msec);

 private:
  // 对象类型与对象编码的一致性由外部维护
  // 对象的类型
  Type type_;
  // 对象的编码
  Encoding encoding_;
  // 对象的数据
  std::variant<I64, Str, List, Hash, Set, Zset> data_;
  // 该对象最后一次访问的绝对时间，毫秒精度时间戳
  // Object 始终有最后一次访问时间
  int64_t access_time_ = 0;
  ;
  // 该对象过期的绝对时间，毫秒精度时间戳
  // 值为 INT64_MAX 表示永不过期
  int64_t expire_time_ = INT64_MAX;
};

enum class Object::Type { kString, kList, kHash, kSet, kZset };
enum class Object::Encoding { kInt, kRaw, kHashTable, kLinkedList };

inline Object::Object(I64 i64)
    : type_(Type::kString), encoding_(Encoding::kInt), data_(i64) {
  Touch();
}

inline Object::Object(Str str)
    : type_(Type::kString), encoding_(Encoding::kRaw), data_(std::move(str)) {
  Touch();
}

inline Object::Object(List list)
    : type_(Type::kList),
      encoding_(Encoding::kLinkedList),
      data_(std::move(list)) {
  Touch();
}

inline Object::Object(Hash hash)
    : type_(Type::kHash),
      encoding_(Encoding::kHashTable),
      data_(std::move(hash)) {
  Touch();
}

inline Object::Object(Set set)
    : type_(Type::kHash),
      encoding_(Encoding::kHashTable),
      data_(std::move(set)) {
  Touch();
}

inline Object::Object(Zset zset)
    : type_(Type::kHash),
      encoding_(Encoding::kHashTable),
      data_(std::move(zset)) {
  Touch();
}

inline Object::I64 Object::i64() const {
  assert(type_ == Type::kString);
  assert(encoding_ == Encoding::kInt);
  return std::get<I64>(data_);
}

inline const Object::Str& Object::str() const {
  assert(type_ == Type::kString);
  assert(encoding_ == Encoding::kRaw);
  return std::get<Str>(data_);
}

inline const Object::List& Object::list() const {
  assert(type_ == Type::kList);
  assert(encoding_ == Encoding::kLinkedList);
  return std::get<List>(data_);
}

inline const Object::Hash& Object::hash() const {
  assert(type_ == Type::kHash);
  assert(encoding_ == Encoding::kHashTable);
  return std::get<Hash>(data_);
}

inline const Object::Set& Object::set() const {
  assert(type_ == Type::kSet);
  assert(encoding_ == Encoding::kHashTable);
  return std::get<Set>(data_);
}

inline const Object::Zset& Object::zset() const {
  assert(type_ == Type::kZset);
  assert(encoding_ == Encoding::kHashTable);
  return std::get<Zset>(data_);
}

inline void Object::set_i64(I64 i64) {
  assert(type_ == Type::kString);
  assert(encoding_ == Encoding::kInt);
  data_ = i64;
}

inline void Object::set_str(Str str) {
  assert(type_ == Type::kString);
  assert(encoding_ == Encoding::kRaw);
  data_ = std::move(str);
}

inline void Object::set_list(List list) {
  assert(type_ == Type::kList);
  assert(encoding_ == Encoding::kLinkedList);
  data_ = std::move(list);
}

inline void Object::set_hash(Hash hash) {
  assert(type_ == Type::kHash);
  assert(encoding_ == Encoding::kHashTable);
  data_ = std::move(hash);
}

inline void Object::set_set(Set set) {
  assert(type_ == Type::kSet);
  assert(encoding_ == Encoding::kHashTable);
  data_ = std::move(set);
}

inline void Object::set_zset(Zset zset) {
  assert(type_ == Type::kZset);
  assert(encoding_ == Encoding::kHashTable);
  data_ = std::move(zset);
}

inline void Object::set_type(Type type) {
  switch (type) {
    case Type::kString:
      encoding_ = Encoding::kRaw;
      data_ = Str{};
    case Type::kList:
      encoding_ = Encoding::kLinkedList;
      data_ = List{};
    case Type::kHash:
      encoding_ = Encoding::kHashTable;
      data_ = Hash{};
    case Type::kSet:
      encoding_ = Encoding::kHashTable;
      data_ = Set{};
    case Type::kZset:
      encoding_ = Encoding::kHashTable;
      data_ = Zset{};
  }
}

inline std::string Object::TypeStr() const {
  switch (type_) {
    case Type::kString:
      return "string";
    case Type::kList:
      return "list";
    case Type::kHash:
      return "hash";
    case Type::kSet:
      return "set";
    case Type::kZset:
      return "zset";
  }
}

inline std::string Object::EncodingStr() const {
  switch (encoding_) {
    case Encoding::kInt:
      return "int";
    case Encoding::kRaw:
      return "raw";
    case Encoding::kHashTable:
      return "hashtable";
    case Encoding::kLinkedList:
      return "linkedlist";
  }
}

}  // namespace mydss::db

#endif  // MYDSS_INCLUDE_DB_OBJECT_HPP_
