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

#include <gtest/gtest.h>

#include <cerrno>
#include <err/errno.hpp>

namespace mydss::err {

TEST(TestErrno, ErrnoStrSuccess) {
  errno = 0;
  auto s = ErrnoStr();
  EXPECT_EQ(s, "Success");
}

}  // namespace mydss::err
