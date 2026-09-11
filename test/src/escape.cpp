#include "check.hpp"

#include <string>
#include <string_view>
#include <variant>

#include <vs-xml/serializer.hpp>

using namespace xml::serialize;

static std::string expand(const ret_t& r){
    if(!r.has_value())return "<invalid>";
    if(std::holds_alternative<std::string>(*r))return std::get<std::string>(*r);
    return std::string(std::get<std::string_view>(*r));
}

static std::string unescape(std::string_view s){
    std::string out;
    for(char c : unescaped_view(s)) out.push_back(c);
    return out;
}

static std::string escape(std::string_view s){
    std::string out;
    for(char c : escaped_view(s)) out.push_back(c);
    return out;
}

int main(){
    // to_xml_text must not lose characters around brackets.
    CHECK(expand(to_xml_text("plain")) == "plain");
    CHECK(expand(to_xml_text("a]b")) == "a]b");
    CHECK(expand(to_xml_text("a]]b")) == "a]]b");
    CHECK(expand(to_xml_text("x]]y]]z")) == "x]]y]]z");
    CHECK(expand(to_xml_text("]]>")) == "]]&gt;");
    CHECK(expand(to_xml_text("a]]>b")) == "a]]&gt;b");
    CHECK(expand(to_xml_text("a<b&c")) == "a&lt;b&amp;c");
    CHECK(expand(to_xml_text("x]]>y<z&w")) == "x]]&gt;y&lt;z&amp;w");

    // cdata/comment/proc reject disallowed sequences.
    CHECK(!to_xml_cdata("a]]>b").has_value());
    CHECK(to_xml_cdata("a]b").has_value());
    CHECK(!to_xml_comment("a--b").has_value());
    CHECK(to_xml_comment("a-b").has_value());
    CHECK(!to_xml_proc("a?>b").has_value());

    // Named and numeric unescaping; both paths agree on the single-byte range.
    CHECK(unescape("A&amp;B &lt;C&gt;&apos;D&quot;E") == "A&B <C>'D\"E");
    CHECK(unescape("&#65;&#x42;") == "AB");
    CHECK(unescape("&#x1F600;") == "&#x1F600;"); // out of range stays literal
    CHECK(unescape("&#;") == "&#;");             // malformed stays literal

    // escaped_view expands the named entities.
    CHECK(escape("A&B<C>") == "A&amp;B&lt;C&gt;");

    // escape_xml performs the same expansion eagerly.
    CHECK(escape_xml("A&B<C>\"D'") == "A&amp;B&lt;C&gt;&quot;D&apos;");

    return 0;
}
