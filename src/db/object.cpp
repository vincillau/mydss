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

#include <sys/time.h>

#include <db/object.hpp>

namespace mydss::db {

namespace {

static int64_t TimeInMsec() {
  timeval now;
  gettimeofday(&now, nullptr);
  int64_t ts = now.tv_sec * 1000 + now.tv_usec / 1000;
  return ts;
}

}  // namespace

void Object::Touch() { access_time_ = TimeInMsec(); }

int64_t Object::pttl() const {
  if (expire_time_ == INT64_MAX) {
    return -1;  // 永不过期
  }
  int64_t now = TimeInMsec();
  if (expire_time_ <= now) {
    return 0;
  }
  return expire_time_ - now;
}

void Object::set_pttl(int64_t msec) {
  assert(msec >= -1);

  if (msec == -1) {
    expire_time_ = INT64_MAX;
    return;
  }
  int64_t now = TimeInMsec();
  expire_time_ = now + msec;
}

}  // namespace mydss::db
