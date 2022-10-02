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

#ifndef MYDSS_INCLUDE_COMMAND_STRING_HPP_
#define MYDSS_INCLUDE_COMMAND_STRING_HPP_

#include <instance.hpp>
#include <resp/req.hpp>
#include <status.hpp>

namespace mydss ::string {

[[nodiscard]] Status Set(const Instance& inst, const Req& req,
                         std::shared_ptr<Piece>& piece);

}  // namespace mydss::string

#endif // MYDSS_INCLUDE_COMMAND_STRING_HPP_
