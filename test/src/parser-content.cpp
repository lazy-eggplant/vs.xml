// Characterization of the parser's text/whitespace handling.

#include "check.hpp"

#include <span>
#include <string>
#include <string_view>

#include <vs-xml/parser.hpp>
#include <vs-xml/tree-builder.hpp>

using namespace xml;

static std::string value_of(wrp::base_t<unknown_t> n){
    auto v = n.value();
    if(!v.has_value())return {};
    return std::string(std::string_view(*v));
}

int main(){
    // Whitespace-only text between elements is ignorable formatting and is dropped.
    {
        std::string xml = "<a>   <b/>\n   </a>";
        TreeBuilder<builder_config_t{.symbols=builder_config_t::COMPRESS_ALL}> b;
        Parser p(std::span<char>(xml.data(), xml.size()), b);
        CHECK(p.parse().has_value());
        auto t = b.close();
        CHECK(t.has_value());
        auto root = t->root();
        size_t count = 0;
        for(auto& c : root.children()){ (void)c; ++count; }
        CHECK(count == 1);
        auto first = *root.children().begin();
        CHECK(first.name().value_or("") == "b");
    }

    // Whitespace that is part of a non-empty text node is preserved.
    {
        std::string xml = "<p>Hello <b>world</b>!</p>";
        TreeBuilder<builder_config_t{.symbols=builder_config_t::COMPRESS_ALL}> b;
        Parser p(std::span<char>(xml.data(), xml.size()), b);
        CHECK(p.parse().has_value());
        auto t = b.close();
        CHECK(t.has_value());
        auto root = t->root();

        auto it = root.children().begin();
        auto end = root.children().end();

        CHECK(it != end);
        auto c0 = *it; ++it;
        CHECK(c0.type() == type_t::TEXT);
        CHECK(value_of(c0) == "Hello ");

        CHECK(it != end);
        auto c1 = *it; ++it;
        CHECK(c1.type() == type_t::ELEMENT);
        CHECK(c1.name().value_or("") == "b");

        CHECK(it != end);
        auto c2 = *it; ++it;
        CHECK(c2.type() == type_t::TEXT);
        CHECK(value_of(c2) == "!");

        CHECK(it == end);
    }

    // Leading whitespace of a text node is kept.
    {
        std::string xml = "<p>  leading</p>";
        TreeBuilder<builder_config_t{.symbols=builder_config_t::COMPRESS_ALL}> b;
        Parser p(std::span<char>(xml.data(), xml.size()), b);
        CHECK(p.parse().has_value());
        auto t = b.close();
        CHECK(t.has_value());
        auto root = t->root();
        auto first = *root.children().begin();
        CHECK(value_of(first) == "  leading");
    }

    // An element that is never closed is an error, not a silent success.
    {
        std::string xml = "<a>";
        TreeBuilder<builder_config_t{.symbols=builder_config_t::COMPRESS_ALL}> b;
        Parser p(std::span<char>(xml.data(), xml.size()), b);
        auto r = p.parse();
        CHECK(!r.has_value());
        CHECK(r.error().code == decltype(r)::error_type::UNEXPECTED_EOF);
    }

    return 0;
}
