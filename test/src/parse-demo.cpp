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
    mio::mmap_source mmap("./test/assets/demo-0.xml");
    std::span<char> w((char*)mmap.data(),mmap.size());

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

    xml::BuilderCompressed builder;
    try {
        xml::Parser parser(xmlData, builder);
        parser.parse();
    } catch (const std::exception &ex) {
        std::cerr << "Error while parsing XML: " << ex.what() << "\n";
        return 1;
    }
    auto tree = *builder.close();
    tree.print(std::cout,{});

    //std::ofstream file("./test/assets/out.bin",std::ios::binary|std::ios::out);
    //    tree.save_binary(file);
    //file.close();
    return 0;
}