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

#ifndef MYDSS_INCLUDE_RESP_PIECE_HPP_
#define MYDSS_INCLUDE_RESP_PIECE_HPP_

#include <memory>
#include <string>
#include <vector>

namespace mydss::proto {

class Piece {
 public:
  // 计算序列化后的大小
  [[nodiscard]] virtual size_t Size() const = 0;

  // 序列化 Piece
  // len 不能小于 Size() 的返回值
  // 返回序列后的数据大小，与 Size() 的返回值相同
  virtual size_t Serialize(char* buf, size_t len) const = 0;
};

class SimpleStringPiece : public Piece {
 public:
  explicit SimpleStringPiece(std::string value) : value_(std::move(value)) {}

  [[nodiscard]] const auto& value() const { return value_; }
  [[nodiscard]] auto& value() { return value_; }

  [[nodiscard]] size_t Size() const override;
  size_t Serialize(char* buf, size_t len) const override;

 private:
  std::string value_;
};

class ErrorPiece : public Piece {
 public:
  explicit ErrorPiece(std::string value) : value_(std::move(value)) {}

  [[nodiscard]] const auto& value() const { return value_; }
  [[nodiscard]] auto& value() { return value_; }

  [[nodiscard]] size_t Size() const override;
  size_t Serialize(char* buf, size_t len) const override;

 private:
  std::string value_;
};

class IntegerPiece : public Piece {
 public:
  explicit IntegerPiece(int64_t value) : value_(value) {}

  [[nodiscard]] const auto& value() const { return value_; }
  [[nodiscard]] auto& value() { return value_; }

  [[nodiscard]] size_t Size() const override;
  size_t Serialize(char* buf, size_t len) const override;

 private:
  int64_t value_;
};

class BulkStringPiece : public Piece {
 public:
  BulkStringPiece() : null_(true) {}
  explicit BulkStringPiece(std::string value)
      : value_(std::move(value)), null_(false) {}

  [[nodiscard]] const auto& value() const { return value_; }
  [[nodiscard]] auto& value() { return value_; }

  [[nodiscard]] size_t Size() const override;
  size_t Serialize(char* buf, size_t len) const override;

 private:
  std::string value_;
  bool null_;
};

class ArrayPiece : public Piece {
 public:
  [[nodiscard]] const auto& pieces() const { return pieces_; }
  [[nodiscard]] auto& pieces() { return pieces_; }

  [[nodiscard]] size_t Size() const override;
  size_t Serialize(char* buf, size_t len) const override;

 private:
  std::vector<std::shared_ptr<Piece>> pieces_;
};

}  // namespace mydss::proto

#endif  // MYDSS_INCLUDE_RESP_PIECE_HPP_
