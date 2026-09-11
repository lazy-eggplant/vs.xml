package("vs.xml")
    set_homepage("https://github.com/lazy-eggplant/vs.xml")
    set_description("High performance XML parser, tree builder and query system for large trees with limited mutability")
    set_license("LGPL-3.0")

    add_urls("https://github.com/lazy-eggplant/vs.xml.git")

    on_install(function (package)
        import("package.tools.xmake").install(package)
    end)

    on_test(function (package)
        assert(package:check_cxxsnippets({test = [[
            #include <vs.xml/tree.hpp>
            void test() {}
        ]]}, {configs = {languages = "c++23"}}))
    end)
