#include <iostream>
#include <string_view>
#include <vs-xml/builder.hpp>
using namespace xml;

int main(){
    //Builder preserves the 
    TreeBuilder<{.compress_symbols=true}> bld;

    //These will later be expanded in nodes.
    std::string_view texts[] = {"Text 0", "Text 1", "Text 2"};

    bld.begin("root","s");
        //Attributes must all come first after a `begin`
        bld.attr("attr-0", "value","s");
        bld.attr("attr-0", "value");
        //We now start adding children
        bld.comment("This is a comment! <escape> sequences will be handled.");
        bld.text("This is some text! <escape> sequences will be handled.");
        bld.cdata("This is some cdata! <escape> sequences will be handled.");
        for(auto& txt: texts){
            bld.begin("child");
                bld.text(txt);
            bld.end();            
        }
    bld.end();

    auto tree = bld.close();
    tree->print(std::cout);
    return 0;
}