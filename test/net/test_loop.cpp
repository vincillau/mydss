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

#include <net/loop.hpp>

namespace mydss::net {

// 测试 Loop 的构造函数
TEST(TestLoop, New) {
  auto loop = Loop::New();
  EXPECT_GE(loop->epfd_, 0);
  EXPECT_TRUE(loop->fd_cbs_map_.empty());
}

// 当事件循环中没有需要监听的文件时 Loop::Run 应该返回
TEST(TestLoop, RunExit) {
  auto loop = Loop::New();
  loop->Run();
  SUCCEED();
}

}  // namespace mydss::net
