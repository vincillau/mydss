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

target("test_err_errno")
    set_kind("binary")
    set_group("test")

    add_files("test_errno.cpp")
    add_includedirs("$(projectdir)/include")

    add_deps("mydss_", "test_main")
    add_links("mydss_", "test_main")
    add_packages("gtest")

target("test_err_status")
    set_kind("binary")
    set_group("test")

    add_files("test_status.cpp")
    add_includedirs("$(projectdir)/include")

    add_deps("mydss_", "test_main")
    add_links("mydss_", "test_main")
    add_packages("fmt", "gtest")
