#include <fstream>
#include <iostream>
#include <ostream>
#include <print>

#include <vs-xml/parser.hpp>
#include <vs-xml/serializer.hpp>
#include <vs-xml/builder.hpp>

#include <mio/mmap.hpp>

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

    xml::TreeBuilder<{.compress_symbols=true}> builder;
    try {
        xml::Parser parser(xmlData, builder);
        parser.parse();
    } catch (const std::exception &ex) {
        std::cerr << "Error while parsing XML: " << ex.what() << "\n";
    }
    auto tree = *builder.close();
    tree.print(std::cout,{});

    mio::mmap_source mmap("./assets/local/demo-0.xml");
    std::span<char> w((char*)mmap.data(),mmap.size());

    xml::TreeBuilder<{.compress_symbols=true}> builder2;
    builder.begin("wrapper");
        builder2.inject(tree);
    builder2.end();
    auto tree2 = builder2.close();

    {
        std::ofstream file("./assets/local/demo-0.bin",std::ios::binary|std::ios::out);
            tree.save_binary(file);
        file.close();
    }

    std::print("\n~~~~\n");

    {
        mio::mmap_source mmap("./assets/local/demo-0.bin");
        auto tmp = xml::TreeRaw::from_binary({.raw_strings=true},std::span((uint8_t*)mmap.data(),mmap.size()));
        tmp.print(std::cout,{});
    }

    std::print("\n~~~~\n");

    return 0;
}