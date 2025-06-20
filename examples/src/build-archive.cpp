#include <iostream>
#include <print>
#include <string_view>
#include <sstream>
#include <vs-xml/archive.hpp>
//#include <vs-xml/archive-builder.hpp>
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
    DocBuilder<{.symbols=xml::builder_config_t::COMPRESS_ALL,.raw_strings=true}> bld;

    //Optionally, you can reserve space for the containers used by the builder. This way you can cut down on allocations.
    bld.reserve(1024*4,1024*4);

    constexpr std::string_view texts[] = {"Text 0", "Text 11", "Text 222"};

    std::vector<binary_header_t::section_t> fragments;
    fragments.reserve(sizeof(texts)/sizeof(std::string_view));


    //Here we constructs multiple documents but we preserve the table of symbols.
    //Please notice the usage of `close_frame` and the final `extract_symbols` in place of the "normal" `close`.

    std::print("Constructing frames.\n");

    for(auto text : texts){
        std::print("Building {}\n",text);
        bld.xml();
        bld.begin("tag-root");
            bld.attr("doc","document-value");
            if(text=="Text 0")bld.attr("doc2","document-value2");
            bld.text(text);
        bld.end();

        //Register the current frame in a vector for later usage.
        if(auto t = bld.close_frame(text); t.has_value())fragments.emplace_back(*t);
        else{
            std::print(std::cerr,"Unable to complete construction of the tree {}",(int)t.error());
            exit(1);
        }

    }

    //Extract symbols once all documents have been built.
    std::print("Extracting symbols.\n");
    auto [buffer,symbols] = *bld.extract();

    /*if(!symbols.has_value()){
        std::print(std::cerr,"Unable to complete construction of the tree; failure to handle symbols.");
        exit(1);
    }*/


    //We now serialize the archive after creation onto a stream, just for demonstrative purposes.
    std::print("Creating archive.\n");
    std::stringstream memstream;

    stored::Archive archive(builder_config_t{.symbols=xml::builder_config_t::COMPRESS_ALL,.raw_strings=true},std::move(fragments),std::move(buffer),std::move(symbols));

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