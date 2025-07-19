#include <string_view>
#include <vs-xml/tree-builder.hpp>
#include <sstream>

template<typename T>
int test(std::function<void(T& bld)>&& fn, std::string_view value){
    xml::TreeBuilder<{}> bld;

    fn(bld);

    auto tree = bld.close();

    if(!tree.has_value()){
        return 1;
    }

    std::string buffer;
    std::stringstream out(buffer);
    
    tree->print(out,{});

    if(out.str()!=value){
        return 2;
    }

    return 0;
}

int main(){
    assert(test<xml::TreeBuilder<{}>>([](auto& bld){},"")==0);

    assert(test<xml::TreeBuilder<{}>>([](auto& bld){
        bld.begin("root","ns");
        bld.end();
    },"<ns:root/>")==0);

    assert(test<xml::TreeBuilder<{}>>([](auto& bld){
        bld.begin("root","ns");
        bld.attr("name1","value1","ns");
        bld.attr("name2","value2");
        bld.end();
    },R"(<ns:root ns:name1="value1" name2="value2"/>)")==0);

    return 0;
}