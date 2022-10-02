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

namespace mydss {

class Piece {
 public:
  using Type = char;

  static constexpr Type kSimpleString = '+';
  static constexpr Type kError = '-';
  static constexpr Type kInteger = ':';
  static constexpr Type kBulkString = '$';
  static constexpr Type kArray = '*';

  [[nodiscard]] virtual Type type() const = 0;
  [[nodiscard]] virtual std::string ToString() const = 0;
};

class SimpleStringPiece : public Piece {
 public:
  explicit SimpleStringPiece(std::string value = {})
      : value_(std::move(value)) {}

  [[nodiscard]] Type type() const override { return Piece::kSimpleString; }
  [[nodiscard]] const auto& value() const { return value_; }
  [[nodiscard]] auto& value() { return value_; }

  [[nodiscard]] std::string ToString() const override {
    return Piece::kSimpleString + value_ + "\r\n";
  }

 private:
  std::string value_;
};

class ErrorPiece : public Piece {
 public:
  explicit ErrorPiece(std::string value = {}) : value_(std::move(value)) {}

  [[nodiscard]] Type type() const override { return Piece::kError; }
  [[nodiscard]] const auto& value() const { return value_; }
  [[nodiscard]] auto& value() { return value_; }

  [[nodiscard]] std::string ToString() const override {
    return Piece::kError + value_ + "\r\n";
  }

 private:
  std::string value_;
};

class IntegerPiece : public Piece {
 public:
  explicit IntegerPiece(std::uint64_t value = 0) : value_(value) {}

  [[nodiscard]] Type type() const override { return Piece::kInteger; }
  [[nodiscard]] const auto& value() const { return value_; }
  [[nodiscard]] auto& value() { return value_; }

  [[nodiscard]] std::string ToString() const override {
    return Piece::kInteger + std::to_string(value_) + "\r\n";
  }

 private:
  std::uint64_t value_;
};

class BulkStringPiece : public Piece {
 public:
  BulkStringPiece() : null_(true) {}
  explicit BulkStringPiece(std::string value)
      : value_(std::move(value)), null_(false) {}

  [[nodiscard]] Type type() const override { return Piece::kBulkString; }
  [[nodiscard]] const auto& value() const { return value_; }
  [[nodiscard]] auto& value() { return value_; }

  [[nodiscard]] std::string ToString() const override {
    if (null_) {
      return Piece::kBulkString + std::string("-1\r\n");
    }
    return Piece::kBulkString + std::to_string(value_.size()) + "\r\n" +
           value_ + "\r\n";
  }

 private:
  std::string value_;
  bool null_;
};

class ArrayPiece : public Piece {
 public:
  [[nodiscard]] Type type() const override { return Piece::kArray; }
  [[nodiscard]] const auto& pieces() const { return pieces_; }
  [[nodiscard]] auto& pieces() { return pieces_; }

  [[nodiscard]] std::string ToString() const override {
    std::string str{Piece::kArray};
    str += std::to_string(pieces_.size());
    for (const auto& piece : pieces_) {
      str += piece->ToString();
    }
    return str;
  }

 private:
  std::vector<std::shared_ptr<Piece>> pieces_;
};

}  // namespace mydss

#endif  // MYDSS_INCLUDE_RESP_PIECE_HPP_
