#include "vs-xml/commons.hpp"
#include "vs-xml/impl.hpp"
#include <ranges>
#include <vs-xml/builder.hpp>

#include <print>
#include <iostream>

template<xml::builder_config_t cfg>
auto mk_tree(){
    xml::TreeBuilder<cfg> build;
    build.begin("hello");
        build.attr("op3-a", "v'>&al1");
        build.attr("op1-a", "val1", "w");
        build.attr("op2-a", "val\"1");
        build.attr("op5-a", "val\"1");
        build.attr("op6-a", "val1", "w");
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
    build.end();

    return build.close();
}

/*
struct it_test {
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = int;           // Use int since we dereference to a value.
    using pointer           = const int*;
    using reference         = const int&;

    it_test() = default;
    it_test(int idx)  {}    //This is what is missing somehow.

    inline reference operator*() const {return {};}
    inline pointer operator->() const {return {};}

    inline it_test& operator++() {return *this;}
    inline it_test operator++(int) {return {};}

    inline it_test& operator--() {return *this;}
    inline it_test operator--(int) {return {};}

    friend bool operator==(const it_test& a, const it_test& b) {return false;}
    friend bool operator!=(const it_test& a, const it_test& b) {return false;}
};

struct container_test : public std::ranges::view_interface<container_test> {
    it_test begin() const { return {}; }
    it_test end() const { return {}; }
};
*/


template<xml::builder_config_t cfg>
auto test(){
    auto tree = *mk_tree<cfg>();
    tree.print(std::cout,{});
    std::print("\n---\n");
    tree.reorder();
    tree.print(std::cout,{});
    std::print("\n---\n");
    xml::Tree wrp_tree(std::move(tree));
    wrp_tree.print(std::cout,{});
    std::print("\n---\n");

    auto root = wrp_tree.root();
    for(auto& attr:root.attrs()){
        std::print("{}\n",attr.name().value_or("--"));
    }

    for(auto& element:root.children()){
        std::print("{}\n",element.name().value_or("--"));
    }

    auto root2 = wrp_tree.downgrade().root();


    auto fn= [](const xml::attr_iterator& i){return true;};
    
    static_assert(std::bidirectional_iterator<xml::attr_iterator>);

/*
    for(auto& element: root.attrs() | std::views::filter([](auto){return true;})){
        std::print("{}\n",element.name().value_or("--"));
    }
*/
 
    std::print("\n");
}


int main(){
    test<{.symbols=xml::builder_config_t::OWNED}>();    
    return 0;
}