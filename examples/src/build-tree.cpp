#include <iostream>
#include <vs-xml/document.hpp>
using namespace xml;

/*
    How to build a simple tree:
*/

int main(){
    //Initialize a tree builder. It is going to manage its own memory.
    //Right now there is no variant operating on externally owned buffers will be provided, but it might be added at some point.
    //Builders require you to define a configuration structure as a template argument.
    TreeBuilder<{.symbols=xml::builder_config_t::COMPRESS_ALL}> bld;

    //Optionally, you can reserve space for the containers used by the builder. This way you can cut down on allocations.
    bld.reserve(1024*4,1024*4);

    //These will later be expanded in nodes.
    std::string_view texts[] = {"Text 0", "Text 1", "Text 2"};

    //Elements and attributes allow for the use of namespaces, optionally assumed to be ""
    bld.begin("root","s");
        //Attributes must all come first right after a `begin` clause
        bld.attr("attr-0", "value","s");
        bld.attr("attr-0", "value");
        //We now start adding children
        bld.comment("This is a comment! <escape> sequences will be handled.");
        bld.text("This is some text! <escape> sequences will be handled.");
        bld.cdata("This is some cdata! <escape> sequences will be handled.");
        bld.proc("php");
        //Feel free to rewrite this cycle as a map lambda in modern C++
        for(auto& txt: texts){
            bld.begin("child");
                bld.text(txt);
            bld.end();            
        }
        //`x` is a shorthand to add an element, optional attributes and optional children all in one statement.
        //There are few overloads for this function, make sure to pick the one most suitable.
        bld.x("tag",{{"attr-1","value-1"}}, [](auto& bld)static{
            bld.comment("Hello world!");
        });
    bld.end();

    //Close will materialize the document tree.
    auto tree = bld.close();

    //For this demo, we are just going to show the serialized version of this document on the standard output.
    tree->print(std::cout);
    std::cout<<"\n";
    
    return 0;
}