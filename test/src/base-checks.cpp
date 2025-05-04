//#include <vs-xml/vs-xml.hpp>
#include <vs-xml/builder.hpp>

#include <print>
#include <iostream>

int main(){
    xml::BuilderCompressed build;

    build.begin("hello");
        build.attr("op3", "v'>&al1");
        build.attr("op1", "val1", "w");
        build.attr("op2", "val\"1");
        build.attr("op5", "val\"1");
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
    build.end();
    auto tree = *build.close();
    tree.print(std::cout,{});
    std::print("\n");
    tree.reorder();
    tree.print(std::cout,{});
    std::print("\n");
    return 0;
}