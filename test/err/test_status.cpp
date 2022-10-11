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

#include <err/code.hpp>
#include <err/status.hpp>

namespace mydss::err {

TEST(TestStatus, Contructor) {
  Status status(kOk, "ok");
  EXPECT_EQ(status.code(), kOk);
  EXPECT_EQ(status.msg(), "ok");
}

TEST(TestStatus, OkAndError1) {
  Status status(kOk, "ok");
  EXPECT_TRUE(status.ok());
  EXPECT_FALSE(status.error());
}

TEST(TestStatus, OkAndError2) {
  Status status(kEof, "end of file");
  EXPECT_FALSE(status.ok());
  EXPECT_TRUE(status.error());
}

TEST(TestStatus, Ok) {
  Status status = Status::Ok();
  EXPECT_EQ(status.code(), kOk);
  EXPECT_EQ(status.msg(), "ok");
}

TEST(TestStatus, ToString) {
  Status status(kOk, "ok");
  EXPECT_EQ(status.ToString(), "(status:0) ok");
}

}  // namespace mydss::err
