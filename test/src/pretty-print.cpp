// Characterization of the XML pretty-printer (TreeRaw::print_cfg_t).

#include "check.hpp"

#include <sstream>
#include <string>

#include <vs-xml/tree-builder.hpp>

using namespace xml;

static std::string render(const TreeRaw& t, TreeRaw::print_cfg_t cfg = {}){
    std::stringstream ss;
    t.print(ss, cfg);
    return ss.str();
}

int main(){
    TreeBuilder<builder_config_t{.symbols=builder_config_t::COMPRESS_ALL}> b;
    b.begin("root");
        b.begin("a");
        b.end();
        b.begin("b");
            b.text("hello");
        b.end();
        b.comment("c");
    b.end();
    auto t = *b.close();

    // Default: tabs and line feeds.
    CHECK(render(t) == "<root>\n\t<a/>\n\t<b>hello</b>\n\t<!--c-->\n</root>");

    // Spaces instead of tabs.
    {
        TreeRaw::print_cfg_t cfg;
        cfg.use_tabs = false;
        cfg.spaces_in_tab = 2;
        CHECK(render(t, cfg) == "<root>\n  <a/>\n  <b>hello</b>\n  <!--c-->\n</root>");
    }

    // CRLF line endings.
    {
        TreeRaw::print_cfg_t cfg;
        cfg.use_lf = false;
        CHECK(render(t, cfg) == "<root>\r\n\t<a/>\r\n\t<b>hello</b>\r\n\t<!--c-->\r\n</root>");
    }

    // Mixed content is kept inline.
    {
        TreeBuilder<builder_config_t{.symbols=builder_config_t::COMPRESS_ALL}> p;
        p.begin("p");
            p.text("Hello ");
            p.begin("b");
                p.text("world");
            p.end();
            p.text("!");
        p.end();
        auto tp = *p.close();
        CHECK(render(tp) == "<p>Hello <b>world</b>!</p>");
    }

    // Single-quoted attributes escape the active quote.
    {
        TreeBuilder<builder_config_t{.symbols=builder_config_t::COMPRESS_ALL}> x;
        x.begin("x");
            x.attr("a", "v'w");
        x.end();
        auto tx = *x.close();
        TreeRaw::print_cfg_t cfg;
        cfg.use_quote = false;
        CHECK(render(tx, cfg) == "<x a='v&apos;w'/>");
    }

    return 0;
}
