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

#ifndef MYDSS_INCLUDE_UTIL_SLICE_HPP_
#define MYDSS_INCLUDE_UTIL_SLICE_HPP_

#include <cassert>
#include <memory>

namespace mydss::util {

class Slice {
 public:
  Slice() = default;
  explicit Slice(size_t size)
      : data_(new char[size]), cap_(size), start_(0), end_(size) {}
  Slice(const Slice& other, size_t start, size_t end)
      : data_(other.data_),
        cap_(other.cap_),
        start_(other.start_ + start),
        end_(other.start_ + end) {
    assert(start_ <= end_);
    assert(other.end_ >= end_);
  }

  [[nodiscard]] char* data() const {
    if (data_ == nullptr) {
      return nullptr;
    }
    return data_.get() + start_;
  }
  [[nodiscard]] size_t size() const { return end_ - start_; }
  [[nodiscard]] bool empty() const { return size() == 0; }

 private:
  std::shared_ptr<char[]> data_ = nullptr;
  size_t cap_ = 0;
  size_t start_ = 0;
  size_t end_ = 0;
};

}  // namespace mydss::util

#endif  // MYDSS_INCLUDE_UTIL_SLICE_HPP_
