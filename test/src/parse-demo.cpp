#include <fstream>
#include <iostream>
#include <ostream>
#include <vs-xml/fwd/print.hpp>

#include <stdexcept>
#include <vs-xml/parser.hpp>
#include <vs-xml/serializer.hpp>
#include <vs-xml/tree-builder.hpp>

//-----------------------------------------------------
// Example main (for testing purposes)
//-----------------------------------------------------
int main() {

    //The parser can mutate this string. I will have to identify if there is any better ownership mechanism here to use
    std::string xmlData = R"(
<ns1:hello op3-a="v'>&amp;al1" op1-a="val1" ns2:op2-a="val&quot;1" op5-a="val&quot;1" op6-a="val1">
    <hello1 ns='s'/>
    <hello2 ns="s">Banana &lt;hello ciao=&quot;worldo&quot; &amp; &gt;</hello2>
    <ns3:hello3 ns="s">
        <!--hello-->
        <hello5 ns="s" op3="val1" ns4:op2="val11" op1="val1"><![CDATA[Hello'''''&&&& world!]]></hello5>
    </ns3:hello3>
    <hello4 ns="s"/>
</ns1:hello>
)";

    xml::TreeBuilder<xml::builder_config_t{.symbols=xml::builder_config_t::COMPRESS_ALL}> builder;
    try {
        xml::Parser parser(std::span<char>(xmlData), builder);
        auto ret = parser.parse();
        if(!ret.has_value())throw std::runtime_error(std::string(ret.error().msg()));
    } catch (const std::exception &ex) {
        std::cerr << "Error while parsing XML: " << ex.what() << "\n";
    }
    auto tree = *builder.close();
    tree.print(std::cout,{});

    xml::TreeBuilder<xml::builder_config_t{.symbols=xml::builder_config_t::COMPRESS_ALL}> builder2;
    builder2.begin("wrapper");
        builder2.inject(tree);
    builder2.end();
    auto tree2 = builder2.close();
    if(tree2.has_value())tree2->print(std::cout,{});

    return 0;
}