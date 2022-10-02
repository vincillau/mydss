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

#ifndef MYDSS_INCLUDE_UTIL_HPP_
#define MYDSS_INCLUDE_UTIL_HPP_

#include <string>

namespace mydss {

static void StrToUpper(std::string& str) {
  for (char& ch : str) {
    if (ch >= 'a' && ch <= 'z') {
      ch = ch - 'a' + 'A';
    }
  }
};

}  // namespace mydss

#endif  // MYDSS_INCLUDE_UTIL_HPP_
