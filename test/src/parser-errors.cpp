#include "check.hpp"
// Parser error-handling checks: mismatched tags, depth limit, malformed input,
// and trailing whitespace in document mode.

#include <cassert>
#include <expected>
#include <string>
#include <string_view>

#include <vs-xml/document-builder.hpp>
#include <vs-xml/parser.hpp>
#include <vs-xml/tree-builder.hpp>

using namespace xml;

template<typename Builder>
static std::expected<void, typename Parser<Builder>::error_t> run(std::string& xml, Builder& b){
    Parser<Builder> p(std::span<char>(xml.data(), xml.size()), b);
    return p.parse();
}

int main(){
    // A closing tag that does not match the open element must fail.
    {
        std::string xml = "<a></b>";
        TreeBuilder<builder_config_t{.symbols=builder_config_t::COMPRESS_ALL}> b;
        auto r = run(xml, b);
        CHECK(!r.has_value());
        CHECK(r.error().code == decltype(r)::error_type::MISMATCHED_END_TAG);
    }

    // Pathological nesting must be rejected instead of overflowing the stack.
    {
        std::string xml;
        for(int i = 0; i < 1100; i++) xml += "<a>";
        for(int i = 0; i < 1100; i++) xml += "</a>";
        TreeBuilder<builder_config_t{.symbols=builder_config_t::COMPRESS_ALL}> b;
        auto r = run(xml, b);
        CHECK(!r.has_value());
        CHECK(r.error().code == decltype(r)::error_type::DEPTH_EXCEEDED);
    }

    // A document that does not begin with an element must fail.
    {
        std::string xml = "not xml";
        TreeBuilder<builder_config_t{.symbols=builder_config_t::COMPRESS_ALL}> b;
        auto r = run(xml, b);
        CHECK(!r.has_value());
        CHECK(r.error().code == decltype(r)::error_type::MISSING_LT_BEGIN);
    }

    // Document mode must accept trailing whitespace after the root.
    {
        std::string xml = "<doc/>   \n";
        DocumentBuilder<builder_config_t{.symbols=builder_config_t::COMPRESS_ALL}> b;
        Parser p(std::span<char>(xml.data(), xml.size()), b);
        auto r = p.parse();
        CHECK(r.has_value());
    }

    // Matching namespaced tags still parse.
    {
        std::string xml = "<s:a><s:b/></s:a>";
        TreeBuilder<builder_config_t{.symbols=builder_config_t::COMPRESS_ALL}> b;
        auto r = run(xml, b);
        CHECK(r.has_value());
    }

    // Invalid XML names are reported as errors instead of thrown.
    {
        std::string xml = "<1bad/>";
        TreeBuilder<builder_config_t{.symbols=builder_config_t::COMPRESS_ALL}> b;
        auto r = run(xml, b);
        CHECK(!r.has_value());
        CHECK(r.error().code == decltype(r)::error_type::INVALID_NAME);
    }

    return 0;
}
