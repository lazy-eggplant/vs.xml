-- Benchmarks for vs.xml. Loaded from the root xmake.lua when `--with-benchmarks=y`.

target("serialize-big")
    set_kind("binary")
    set_default(false)
    set_rundir(os.projectdir())
    add_files("src/serialize-big.cpp")
    add_deps("vs.xml")
    add_packages("mio", "nanobench", "pugixml")
