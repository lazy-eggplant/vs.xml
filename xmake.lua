--[[
    Build script for vs.xml.

    Primary build system, replacing the legacy Meson and CMake scripts.

    Common invocations:
        xmake f --with-tests=y --with-examples=y --with-tools=y
        xmake
        xmake test

    Options:
        --no_except        build the library without exceptions (library-only)
        --no_assert        disable internal asserts
        --use_gtl          use gtl containers where available (default: false)
        --ns               namespace name (default: xml)
        --binlayout        binary layout variant (default: 0)
        --with_tools       build command line tools (requires mio)
        --with_examples    build the examples
        --with_benchmarks  build the benchmarks
        --with_tests       build the test-suite
]]

local version = os.getenv("VS_XML_VERSION") or "0.3.1"
set_version(version)
set_license("LGPL-3.0-only")

add_rules("plugin.compile_commands.autoupdate")
add_rules("mode.debug", "mode.release")
set_languages("cxx23", "c11")

option("no_except")
    set_default(false)
    set_showmenu(true)
    set_category("Build Options")
    set_description("Build the library with exceptions disabled")
option_end()

option("no_assert")
    set_default(false)
    set_showmenu(true)
    set_category("Build Options")
    set_description("Disable internal asserts")
option_end()

option("use_gtl")
    set_default(false)
    set_showmenu(true)
    set_category("Build Options")
    set_description("Use gtl containers where available")
option_end()

option("ns")
    set_default("xml")
    set_showmenu(true)
    set_category("Build Options")
    set_description("Namespace name")
option_end()

option("binlayout")
    set_default(0)
    set_showmenu(true)
    set_category("Build Options")
    set_description("Binary layout variant")
option_end()

option("with_tools")
    set_default(false)
    set_showmenu(true)
    set_category("Build Options")
    set_description("Build the command line tools (requires mio)")
option_end()

option("with_examples")
    set_default(false)
    set_showmenu(true)
    set_category("Build Options")
    set_description("Build the examples")
option_end()

option("with_benchmarks")
    set_default(false)
    set_showmenu(true)
    set_category("Build Options")
    set_description("Build the benchmarks")
option_end()

option("with_tests")
    set_default(false)
    set_showmenu(true)
    set_category("Build Options")
    set_description("Build the test-suite")
option_end()

add_requires("fmt")

if has_config("use_gtl") then
    add_requires("gtl")
end
if has_config("with_tools") or has_config("with_examples") or has_config("with_benchmarks") then
    add_requires("mio")
end
if has_config("with_tools") then
    add_requires("linenoise")
end
if has_config("with_benchmarks") then
    add_requires("nanobench")
    add_requires("pugixml")
end

if has_config("no_except") and (has_config("with_tools") or has_config("with_examples") or has_config("with_benchmarks") or has_config("with_tests")) then
    raise("Only the base library can be built when `no_except` is enabled")
end

target("vs.xml")
    set_kind("static")
    before_build(function (target)
        local v = version
        local ma, mi, pa = v:match("^(%d+)%.?(%d*)%.?(%d*)")
        local f = assert(io.open(path.join(os.projectdir(), "include/vs-xml/config.hpp"), "w"))
        f:write("#pragma once\n\n")
        f:write("#define VS_XML_VERSION_MAJOR " .. (ma or "0") .. "\n")
        f:write("#define VS_XML_VERSION_MINOR " .. (mi or "0") .. "\n")
        f:write("#define VS_XML_VERSION_REV " .. (pa or "0") .. "\n")
        f:write("#define VS_XML_VERSION \"" .. v .. "\"\n\n")
        f:write("#define VS_XML_NO_EXCEPT " .. (has_config("no_except") and 1 or 0) .. "\n")
        f:write("#define VS_XML_NO_ASSERT " .. (has_config("no_assert") and 1 or 0) .. "\n")
        f:write("#define VS_XML_USE_GTL " .. (has_config("use_gtl") and 1 or 0) .. "\n")
        f:write("#define VS_XML_NS " .. get_config("ns") .. "\n")
        f:write("#define VS_XML_LAYOUT " .. tostring(get_config("binlayout")) .. "\n")
        f:close()
    end)
    add_includedirs("include", {public = true})
    add_headerfiles("include/(vs-xml/**.hpp)", {prefixdir = ""})
    add_headerfiles("include/(vs-xml/c.h)", {prefixdir = ""})
    add_headerfiles("include/(vs-xml/config.hpp)", {prefixdir = ""})
    add_files("lib/**.cpp")
    add_packages("fmt", {public = true})
    if has_config("use_gtl") then
        add_packages("gtl", {public = true})
    end
    if has_config("no_except") then
        add_cxxflags("-fno-exceptions", "-fno-rtti")
    end
    if is_mode("release") then
        set_optimize("fastest")
    end

if has_config("with_tools") then
    includes("tools")
end

if has_config("with_examples") then
    includes("examples")
end

if has_config("with_benchmarks") then
    includes("benchmark")
end

if has_config("with_tests") then
    includes("test")
end
