#include <print>
#include <iostream>
#include <ranges>

#include <vs-xml/commons.hpp>
#include <vs-xml/filters.hpp>
#include <vs-xml/private/impl.hpp>
#include <vs-xml/tree-builder.hpp>


template<xml::builder_config_t cfg>
auto mk_tree(){
    xml::TreeBuilder<cfg> build;
    build.reserve({100000,100000});
    build.begin("hello");
        build.x("AAA",{{"N1","N2"},{"N1","N3"}},[&]{
            build.x("BBB");
            build.comment("ss");
            build.comment("comment2");
        });
        build.text("hello world");
        build.x("a","AAA",{{"N1","N2"},{"N1","N3"}},[](auto& w) static{
            w.x("BBB");
            w.comment("ss");
            w.comment("comment2");
        });
        build.attr("op3-a", "v'>&al1");
        build.attr("op1-a-s", "val1", "w");
        build.attr("op2-a", "va&gt;l\"1");
        build.attr("op5-a", "va>l\"1");
        build.attr("op6-a-s", "val1", "w");
        build.comment("01234567890123456789Banana <hello ciao=\"worldo\" &amp; &></world>"); 
        build.begin("hello1","s");
        build.end();
        build.begin("hello2","s");
            build.text("Banana <hello ciao=\"worldo\" &amp; &></world>"); 
        build.end();
        build.begin("hello3","s");
            build.comment("hello");
            build.begin("hello5","s");
            build.attr("op3", "val1");
            build.attr("op2", "val11");
            build.attr("op1", "val1");
            build.cdata("Hello'''''&&&& world!");
            build.end();
        build.end();
        build.begin("hello4","s");
        
        build.end();
        build.text("ciao mondo");
    build.end();

    return build.close();
}



int main(){
    auto tree = *mk_tree<{.symbols=xml::builder_config_t::OWNED, .raw_strings=true}>();
    for(auto it: tree.root().visitor()){
        std::print(">{}\n",(int)it.type());
    }

    for(auto it: tree.downgrade().root().visitor()){
        std::print(">{}\n",(int)it.type());
    }

    std::print("\n\n");

    xml::TreeRaw::visit(&tree.downgrade().root(),+[](const xml::unknown_t*){return true;});
    xml::Tree::visit(tree.root(),[&](auto node){
        if(node.type()==xml::type_t::ELEMENT && node.name()=="AAA")return false;
        std::print(">{}\n",(int)node.type());
        return true;
    });
    return 0;
}