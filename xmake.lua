set_project("dss")
set_version("0.0.0")
set_languages("c++17")

add_rules("mode.debug", "mode.release")
add_requires("asio", "fmt", "spdlog")

target("mydss")
    set_kind("binary")
    add_includedirs("include")
    add_files("src/**.cpp")
    add_defines("SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG")
    add_packages("asio", "fmt", "spdlog")
