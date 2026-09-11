#include <vs-xml/fwd/print.hpp>
#include <iostream>
#include <ranges>

#include <vs-xml/commons.hpp>
#include <vs-xml/filters.hpp>
#include <vs-xml/tree-builder.hpp>


template<xml::builder_config_t cfg>
auto mk_tree(){
    xml::TreeBuilder<cfg> build;
    build.reserve({100000,100000});
    build.begin("hello");
        build.text("hello world");
        build.x("AAA",{{"N1","N2"},{"N1","N3"}},[&]{
            build.x("BBB");
            build.comment("ss");
            build.comment("comment2");
        });
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




template<xml::builder_config_t cfg>
auto test(){
    auto tree = *mk_tree<cfg>();
    tree.print(std::cout,{});
    fmt::print("\n---\n");
    tree.print_fast(std::cout,{});
    fmt::print("\n---\n");
    tree.reorder();
    tree.print(std::cout,{});
    fmt::print("\n---\n");
    xml::Tree wrp_tree(std::move(tree));
    wrp_tree.print(std::cout,{});
    fmt::print("\n---\n");
    
    auto root = wrp_tree.root();
    
    for(auto& element:root.children()){
        fmt::print("{}\n",element.name().value_or("--"));
    }

    for(auto& element:root.attrs()){
        fmt::print("{}\n",element.name().value_or("--"));
    }
    
    fmt::print("\nNS\n");

    for(auto& element: root.attrs() | xml::filters::ns("w") ){
        fmt::print("{}\n",element.name().value_or("--"));
    }

    for(auto& element: root.attrs(std::views::filter([](auto& it)static{it.type();return true;})) ){
        fmt::print("{}\n",element.name().value_or("--"));
    }
 
    fmt::print("\n === \n");

    for(auto& element: root.attrs(xml::filters::value("va>l\"1")) ){
        fmt::print("{}\n",element.name().value_or("--"));
    }

    fmt::print("\n");

    for(auto& ch: root.text()){fmt::print("{}",ch);}
    fmt::print("\n------\n");
}


int main(){
    test<xml::builder_config_t{.symbols=xml::builder_config_t::OWNED, .raw_strings=true}>();    
    return 0;
}