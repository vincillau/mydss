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

set_project("mydss")
VERSION = "0.1.0"
set_version(VERSION)
set_languages("c++17")

add_rules("mode.debug", "mode.release")
add_requires(
    "fmt >= 9.1.0",
    "gtest >= 1.12.1",
    "nlohmann_json >= 3.11.2",
    "spdlog >= 1.10.0"
)

target("mydss")
    set_kind("binary")
    set_configvar("MYDSS_VERSION", VERSION)
    set_configdir("$(projectdir)/include")
    add_configfiles("include/version.hpp.in")
    add_files("src/*.cpp")
    add_files("src/**/*.cpp")
    add_includedirs("include")
    add_defines("SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG")
    add_packages("fmt", "spdlog", "nlohmann_json")

includes("test")
