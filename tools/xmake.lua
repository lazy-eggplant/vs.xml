-- Command line tools for vs.xml. Loaded from the root xmake.lua when `--with-tools=y`.
-- These are the only targets that depend on mio.

local tools = {
    {name = "vs.xml.encode",         file = "encode.cpp",         packages = {"mio"}},
    {name = "vs.xml.encode-archive", file = "encode-archive.cpp", packages = {"mio"}},
    {name = "vs.xml.decode",         file = "decode.cpp",         packages = {"mio"}},
    {name = "vs.xml.decode-archive", file = "decode-archive.cpp", packages = {"mio"}},
    {name = "vs.xml.query",          file = "query.cpp",          packages = {"mio", "linenoise"}},
}

for _, tool in ipairs(tools) do
    target(tool.name)
        set_kind("binary")
        set_default(false)
        add_files(tool.file)
        add_deps("vs.xml")
        for _, pkg in ipairs(tool.packages) do
            add_packages(pkg)
        end
end
