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

#ifndef MYDSS_INCLUDE_MODULE_LOG_HPP_
#define MYDSS_INCLUDE_MODULE_LOG_HPP_

#include <spdlog/spdlog.h>

#define MYDSS_LOG_DEBUG(...) SPDLOG_DEBUG(__VA_ARGS__)
#define MYDSS_LOG_INFO(...) SPDLOG_INFO(__VA_ARGS__)
#define MYDSS_LOG_WARN(...) SPDLOG_WARN(__VA_ARGS__)
#define MYDSS_LOG_ERROR(...) SPDLOG_ERROR(__VA_ARGS__)

#endif  // MYDSS_INCLUDE_MODULE_LOG_HPP_