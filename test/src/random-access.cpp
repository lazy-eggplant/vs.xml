#include "vs-xml/node.hpp"
#include <iostream>
#include <vs-xml/tree-builder.hpp>
#include <vs-xml/parser.hpp>
#include <vs-xml/utils/rand-access.hpp>

static constexpr const char data[] = R"(
<root>
    <child-0>
        <child-a/>
        <child-b/>
        <child-c/>
    </child-0>
    <child-1/>
    <child-2>
        <child-a/>
        <child-b/>
        <child-c/>
        <child-d/>
    </child-2>
    <child-3/>
</root>
)";

int main(){
    xml::TreeBuilder<{.symbols=xml::builder_config_t::COMPRESS_ALL,.raw_strings=true}> builder;
    try {
        xml::Parser parser(std::span<const char>{data,sizeof(data)}, builder);
        auto ret = parser.parse();
        if(!ret.has_value())throw std::runtime_error(std::string(ret.error().msg()));
    } catch (const std::exception &ex) {
        std::cerr << "Error while parsing XML: " << ex.what() << "\n";
    }
    auto tree = *builder.close();
    //tree.print(std::cout,{});

    RandomAccessIndexLazy idx(tree);
    //idx.reserve({.buffer=1000});

    for(auto item: idx.children(0)){
        auto ppos = idx.resolve_rel(item);
        auto node = idx.resolve((xml::unknown_t*)&tree.downgrade().root(),0+item);
        std::cout<<tree.downgrade().rsv(*node->name())<<"\n";

        for(auto item2: idx.children(0+ppos)){
            auto node2 = idx.resolve(node,item2);
            std::cout<<"\t"<<tree.downgrade().rsv(*node2->name())<<"\n";
        }
    }
    return 0;
}