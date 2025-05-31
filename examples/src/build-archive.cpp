#include <iostream>
#include <print>
#include <string_view>
#include <vs-xml/document.hpp>
using namespace xml;

/*
    Example on how to build a multi-document archive:
    - Split the generation in frames, and extract the symbol table at the very end.
    - Provide them to an Archive instance to serialize.
    - Profit.
*/

int main(){
    DocBuilder<{.symbols=xml::builder_config_t::COMPRESS_ALL,.raw_strings=true}> bld;

    //Optionally, you can reserve space for the containers used by the builder. This way you can cut down on allocations.
    bld.reserve(1024*4,1024*4);

    constexpr std::string_view texts[] = {"Text 0", "Text 1", "Text 2"};

    std::vector<std::pair<std::string_view,std::vector<uint8_t>>> fragments;
    fragments.reserve(sizeof(texts)/sizeof(std::string_view));

    for(auto text : texts){

        bld.xml();
        bld.begin("tag-root");
            bld.attr("doc","document-value");
            bld.text(text);
        bld.end();

        if(auto t = bld.close_frame(); t.has_value())fragments.emplace_back(*t);
        else{
            std::print(std::cerr,"Unable to complete construction of the tree");
            exit(1);
        }

    }

    auto symbols = bld.extract_symbols();
    if(!symbols.has_value()){
        std::print(std::cerr,"Unable to complete construction of the tree; failure to handle symbols.");
        exit(1);
    }

    //TODO: class not implemented yet, missing serialization.
    /*Archive archive;

    //For this demo, we are just going to show the serialized version of this document on the standard output.
    archive->print(std::cout);
    std::cout<<"\n";
    */

    return 0;
}