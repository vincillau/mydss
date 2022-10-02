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

#include <list>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>

namespace mydss {

class Object {
 public:
  using Str = std::string;
  using List = typename std::list<std::string>;
  using Hash = typename std::unordered_map<std::string, std::string>;
  using Set = typename std::unordered_set<std::string>;
  using Zset = typename std::set<std::string>;

  enum class Type { kStr, kList, kHash, kSet, kZset };

  explicit Object(Type type);

  [[nodiscard]] auto type() const { return type_; }

  template <typename T>
  [[nodiscard]] const T* GetIf() const {
    return std::get_if<T>(&data_);
  };

  template <typename T>
  [[nodiscard]] T* GetIf() {
    return std::get_if<T>(&data_);
  };

  void ToStr() {
    type_ = Type::kStr;
    data_ = Str{};
  }

  void ToList() {
    type_ = Type::kList;
    data_ = List{};
  };

  void ToHash() {
    type_ = Type::kHash;
    data_ = Hash{};
  };

  void ToSet() {
    type_ = Type::kSet;
    data_ = Set{};
  };

  void ToZset() {
    type_ = Type::kZset;
    data_ = Zset{};
  };

 private:
  Type type_;
  std::variant<Str, List, Hash, Set, Zset> data_;
};

inline Object::Object(Type type) {
  switch (type) {
    case Type::kStr:
      ToStr();
    case Type::kList:
      ToList();
    case Type::kHash:
      ToHash();
    case Type::kSet:
      ToSet();
    case Type::kZset:
      ToZset();
  }
}

}  // namespace mydss

#endif  // MYDSS_INCLUDE_DB_OBJECT_HPP_
