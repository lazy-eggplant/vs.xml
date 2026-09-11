-- Test-suite for vs.xml. Loaded from the root xmake.lua when `--tests=y`.

local test_names = {
    "tree-builder",
    "base-checks",
    "parse-demo",
    "escape",
    "query",
    "binary-format",
    "tree-iterator",
    "random-access",
    "parser-errors",
    "parser-content",
    "pretty-print",
}

for _, name in ipairs(test_names) do
    target(name)
        set_kind("binary")
        set_default(false)
        set_rundir(os.projectdir())
        add_files("src/" .. name .. ".cpp")
        add_deps("vs.xml")
        add_tests(name)
end

-- Plain C translation unit exercising the bindings.
target("c-interface")
    set_kind("binary")
    set_default(false)
    set_rundir(os.projectdir())
    add_files("c-interface.c")
    add_deps("vs.xml")
    add_tests("c-interface")
