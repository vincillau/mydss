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

#ifndef MYDSS_INCLUDE_MODULE_TYPE_HPP_
#define MYDSS_INCLUDE_MODULE_TYPE_HPP_

#include <cstdint>

namespace mydss::module {

namespace type {

static constexpr uint16_t kUnknown = 0;
static constexpr uint16_t kString = 1;

}  // namespace type

namespace encoding {

static constexpr uint16_t kUnknown = 0;
static constexpr uint16_t kInt = 1;
static constexpr uint16_t kRaw = 2;

}  // namespace encoding

}  // namespace mydss::module

#endif  // MYDSS_INCLUDE_MODULE_TYPE_HPP_
