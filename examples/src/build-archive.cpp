#include "vs-xml/tree-builder.hpp"
#include <iostream>
#include <print>
#include <string_view>
#include <sstream>
#include <vs-xml/archive.hpp>
#include <vs-xml/archive-builder.hpp>
#include <vs-xml/document-builder.hpp>
#include <vs-xml/utils/pretty-header.hpp>
using namespace xml;

/*
    Example on how to build a multi-document archive:
    - Split the generation in frames, and extract the symbol table at the very end.
    - Provide them to an Archive instance to serialize.
    - Profit.
*/


int main(){
    ArchiveBuilder<{.symbols=xml::builder_config_t::COMPRESS_ALL,.raw_strings=true}> bld;
    bld.reserve({1024*4,1024*4});

    constexpr std::string_view texts[] = {"Text 0", "Text 11", "Text 222"};

    std::print("Constructing frames.\n");

    for(auto text : texts){
        if(auto t = bld.document(text, [&](auto& bld){
            bld.xml();
            bld.begin("tag-root");
                bld.attr("doc","document-value");
                if(text=="Text 0")bld.attr("doc2","document-value2");
                bld.text(text);
            bld.end();
        });t!=decltype(t)::OK){
            std::print(std::cerr,"Unable to complete construction of the tree {}",(int)t);
            exit(1);
        }
    }

    std::print("Creating archive.\n");

    stored::Archive archive = *bld.close();
    //TODO: Handle here if not created properly.

    //We now serialize the archive after creation onto a stream, just for demonstrative purposes.
    std::stringstream memstream;


    std::print("Saving as binary.\n");
    if(!archive.save_binary(memstream)){
        std::print(std::cerr, "Failed to properly save the binary file\n");
        exit(1);
    }
  
    memstream.flush();
    
    std::print("Loading from binary.\n");
    
    auto tmp_str = memstream.str();

    //Building an archive this way makes it weak, make sure the buffer lifetime exceeds the one of the archive.
    auto archive_back = Archive::from_binary({(const uint8_t*)tmp_str.data(),(const uint8_t*)tmp_str.data()+tmp_str.length()});

    //Just some fancy display to show its binary form.    
    print_header_colors<32>(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(tmp_str.data()), tmp_str.size()));

    if(!archive_back.has_value()){
        std::print(std::cerr,"{}",archive_back.error().msg());
        exit(3);
    }

    std::print("Serializing documents to XML.\n");

    for(size_t i=0;i<archive_back->items();i++){
        archive_back->get(i)->print(std::cout);
        std::print("\n----\n");
    }

    //If a name was given to the document, it is possible to search for it in place of of using an index via `get`.

    return 0;
}