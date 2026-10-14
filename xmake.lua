set_xmakever("2.8.2")

set_project("EasyOutfit")
set_version("1.0.0")
set_license("MIT")

set_languages("c++23")
set_warnings("allextra")

add_rules("mode.release")
add_rules("plugin.vsxmake.autoupdate")

add_requires("commonlibsse-ng", "xbyak")

target("EasyOutfit")
    set_kind("shared")
    add_packages("commonlibsse-ng", "xbyak")

    add_files("src/*.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")
