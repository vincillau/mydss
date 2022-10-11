-- Copyright 2022 Vincil Lau
--
-- Licensed under the Apache License, Version 2.0 (the "License");
-- you may not use this file except in compliance with the License.
-- You may obtain a copy of the License at
--
--     http://www.apache.org/licenses/LICENSE-2.0
--
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations under the License.

set_project("dss")
set_version("0.1.0")
set_languages("c++17")

add_rules("mode.debug", "mode.release")
add_requires("fmt", "gtest", "nlohmann_json", "spdlog")

target("mydss")
    set_kind("binary")
    add_includedirs("include")
    add_files("src/main.cpp")
    add_defines("SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG")
    add_deps("mydss_")
    add_links("mydss_")
    add_packages("fmt", "spdlog", "nlohmann_json")

target("mydss_")
    set_kind("static")
    add_includedirs("include")
    add_files("src/cmd/*.cpp")
    add_files("src/db/*.cpp")
    add_files("src/net/*.cpp")
    add_files("src/proto/*.cpp")
    add_files("src/util/*.cpp")
    add_defines("SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG")
    add_packages("fmt", "spdlog")

includes("test")
