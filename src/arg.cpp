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

#include <fmt/core.h>
#include <getopt.h>

#include <arg.hpp>
#include <cassert>
#include <err/code.hpp>

using fmt::format;
using mydss::err::kInvalidArgs;
using mydss::err::Status;

namespace mydss {

static constexpr int kHelpVal = 256;
static constexpr int kVersionVal = 257;
static constexpr int kConfigVal = 258;

Status Arg::Parse(int argc, char** argv) {
  opterr = 0;
  struct option opts[4];

  opts[0].name = "help";
  opts[0].has_arg = no_argument;
  opts[0].flag = nullptr;
  opts[0].val = kHelpVal;

  opts[1].name = "version";
  opts[1].has_arg = no_argument;
  opts[1].flag = nullptr;
  opts[1].val = kVersionVal;

  opts[2].name = "config";
  opts[2].has_arg = required_argument;
  opts[2].flag = nullptr;
  opts[2].val = kConfigVal;

  opts[3].name = nullptr;
  opts[3].has_arg = no_argument;
  opts[3].flag = nullptr;
  opts[3].val = 0;

  for (;;) {
    int val = getopt_long(argc, argv, "c:", opts, nullptr);
    if (val == -1) {
      break;
    }

    switch (val) {
      case 'c':
      case kConfigVal:
        conf_file_ = optarg;
        break;
      case kHelpVal:
        // 解析到 --help 选项则立即返回
        help_ = true;
        return Status::Ok();
      case kVersionVal:
        // 解析到 --version 选项则立即返回
        return Status::Ok();
      case '?':
        if (optopt == 'c') {
          return {kInvalidArgs, "-c option requires a argument"};
        } else if (optopt == kConfigVal) {
          return {kInvalidArgs, "--config option requires a argument"};
        } else {
          return {kInvalidArgs,
                  format("unknown option '{}'", static_cast<char>(optopt))};
        }
      default:
        assert(false);
    }
  }

  return Status::Ok();
}

}  // namespace mydss
