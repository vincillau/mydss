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

#include <spdlog/spdlog.h>

#include <instance.hpp>
#include <net/server.hpp>

using asio::io_context;
using asio::steady_timer;
using mydss::Instance;
using mydss::kTimeInterval;
using mydss::Server;
using spdlog::flush_on;
using spdlog::set_level;
using spdlog::level::debug;
using std::bind;
using std::error_code;
using std::make_shared;
using std::shared_ptr;
using std::chrono::seconds;
using std::placeholders::_1;

static void OnTimeout(shared_ptr<steady_timer> timer, const error_code& err) {
  if (err) {
    SPDLOG_CRITICAL("timer error: `({}) {}`", err, err.message());
    exit(EXIT_FAILURE);
  }
  SPDLOG_DEBUG("timer timeout");

  timer->expires_after(seconds(kTimeInterval));
  Instance::GetInstance()->Timeout();
  timer->async_wait(bind(OnTimeout, timer, _1));
}

int main(int argc, char** argv) {
  set_level(debug);
  flush_on(debug);

  auto ctx = make_shared<io_context>();
  Instance::Init();

  auto timer = make_shared<steady_timer>(*ctx);
  timer->expires_after(seconds(kTimeInterval));
  timer->async_wait(bind(OnTimeout, timer, _1));

  Server server("127.0.0.1", 6380, ctx);
  server.Run();

  return 0;
}
