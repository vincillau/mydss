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

#ifndef MYDSS_INCLUDE_COMMAND_GENERIC_HPP_
#define MYDSS_INCLUDE_COMMAND_GENERIC_HPP_

#include <instance.hpp>
#include <resp/req.hpp>
#include <status.hpp>

namespace mydss::generic {

[[nodiscard]] Status Del(Instance& inst, const Req& req,
                         std::shared_ptr<Piece>& piece);

[[nodiscard]] Status Exists(Instance& inst, const Req& req,
                            std::shared_ptr<Piece>& piece);

[[nodiscard]] Status Expire(Instance& inst, const Req& req,
                            std::shared_ptr<Piece>& piece);

[[nodiscard]] Status ExpireAt(Instance& inst, const Req& req,
                              std::shared_ptr<Piece>& piece);

[[nodiscard]] Status Keys(Instance& inst, const Req& req,
                          std::shared_ptr<Piece>& piece);

[[nodiscard]] Status Move(Instance& inst, const Req& req,
                          std::shared_ptr<Piece>& piece);

[[nodiscard]] Status Object(Instance& inst, const Req& req,
                                   std::shared_ptr<Piece>& piece);

[[nodiscard]] Status ObjectEncoding(Instance& inst, const Req& req,
                                    std::shared_ptr<Piece>& piece);

[[nodiscard]] Status ObjectIdleTime(Instance& inst, const Req& req,
                                    std::shared_ptr<Piece>& piece);

[[nodiscard]] Status ObjectRefCount(Instance& inst, const Req& req,
                                    std::shared_ptr<Piece>& piece);

[[nodiscard]] Status Persist(Instance& inst, const Req& req,
                             std::shared_ptr<Piece>& piece);

[[nodiscard]] Status PExpire(Instance& inst, const Req& req,
                             std::shared_ptr<Piece>& piece);

[[nodiscard]] Status PExpireAt(Instance& inst, const Req& req,
                               std::shared_ptr<Piece>& piece);

[[nodiscard]] Status PTtl(Instance& inst, const Req& req,
                          std::shared_ptr<Piece>& piece);

[[nodiscard]] Status Rename(Instance& inst, const Req& req,
                            std::shared_ptr<Piece>& piece);

[[nodiscard]] Status RenameNx(Instance& inst, const Req& req,
                              std::shared_ptr<Piece>& piece);

[[nodiscard]] Status Touch(Instance& inst, const Req& req,
                           std::shared_ptr<Piece>& piece);

[[nodiscard]] Status Ttl(Instance& inst, const Req& req,
                         std::shared_ptr<Piece>& piece);

[[nodiscard]] Status Type(Instance& inst, const Req& req,
                          std::shared_ptr<Piece>& piece);

}  // namespace mydss::generic

#endif  // MYDSS_INCLUDE_COMMAND_GENERIC_HPP_
