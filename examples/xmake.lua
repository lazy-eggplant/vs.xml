-- Examples for vs.xml. Loaded from the root xmake.lua when `--with-examples=y`.

local examples = {
    {name = "readme",            file = "readme.cpp"},
    {name = "tree-building",     file = "build-tree.cpp"},
    {name = "document-building", file = "build-doc.cpp"},
    {name = "archive-building",  file = "build-archive.cpp"},
    {name = "from-xml",          file = "from-xml.cpp", packages = {"mio"}},
    {name = "to-xml",            file = "to-xml.cpp",   packages = {"mio"}},
}

for _, example in ipairs(examples) do
    target(example.name)
        set_kind("binary")
        set_default(false)
        add_files("src/" .. example.file)
        add_deps("vs.xml")
        for _, pkg in ipairs(example.packages or {}) do
            add_packages(pkg)
        end
end
