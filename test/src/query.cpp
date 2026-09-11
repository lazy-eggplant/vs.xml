#include "check.hpp"

#include <string_view>
#include <vs-xml/query.hpp>
#include <vs-xml/tree-builder.hpp>

#include <vs-xml/fwd/print.hpp>

using namespace xml;
using namespace xml::query;

template<xml::builder_config_t cfg>
static auto mk_tree(){
    xml::TreeBuilder<cfg> build;
    build.reserve({100000,100000});
    build.begin("root");
        build.x("node-a",{{"attr0","val0"},{"attr1","val1"}});
        build.x("node-b",{{"attr0","val0"},{"attr1","val1"}});
        build.x("node-a",{{"attr0","val0"},{"attr1","val1"}},[](auto& w) static{
            w.x("node-a",{{"attr0","val0"},{"attr1","val1"}});
        });
        build.begin("hello2","s");
            build.text("Banana <hello ciao=\"worldo\" &amp; &></world>");
        build.end();
    build.end();

    return build.close();
}

int main(){
    auto tree = *mk_tree<xml::builder_config_t{.symbols=xml::builder_config_t::OWNED, .raw_strings=true}>();
    auto root = tree.root();

    // accept() alone yields the root.
    {
        query_t q;
        q.accept();
        auto v = q.collect(root);
        CHECK(v.size() == 1);
        CHECK(v[0].name().value_or("") == "root");
    }

    // Direct children named node-a carrying attr0.
    {
        query_t q;
        q.child().element("node-a").match_attr(any(),eq("attr0"),any()).accept();
        CHECK(q.collect(root).size() == 2);
    }

    // Direct children carrying attr0="val0".
    {
        query_t q;
        q.child().match_attr(any(),eq("attr0"),eq("val0")).accept();
        CHECK(q.collect(root).size() == 3);
    }

    // Descendant-or-self carrying attr0="val0".
    {
        query_t q;
        q.child().fork().match_attr(any(),eq("attr0"),eq("val0")).accept();
        CHECK(q.collect(root).size() == 4);
    }

    // Descendants matching a text node's own value.
    {
        query_t q;
        q.descend().match_type(type_t::TEXT)
         .match_value(eq("Banana <hello ciao=\"worldo\" &amp; &></world>")).accept();
        CHECK(q.collect(root).size() == 1);
    }

    // Text content of an element, via a predicate.
    {
        query_t q;
        q.fork().element("hello2")
         .match_text(filter_t{filter_t::predicate_t{[](std::string_view s){return s.find("Banana") != std::string_view::npos;}}})
         .accept();
        CHECK(q.collect(root).size() == 1);
    }

    // has() early-exit behaviour.
    {
        query_t q;
        q.fork().match_type(type_t::COMMENT);
        CHECK(q.has(root) == false); // the trimmed tree has no comments
    }

    return 0;
}
