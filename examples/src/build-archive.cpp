#include "vs-xml/commons.hpp"
#include <iostream>
#include <print>
#include <string_view>
#include <sstream>
#include <vs-xml/archive.hpp>
using namespace xml;

/*
    Example on how to build a multi-document archive:
    - Split the generation in frames, and extract the symbol table at the very end.
    - Provide them to an Archive instance to serialize.
    - Profit.
*/

// ANSI escape code definitions for colors
namespace colors {
    constexpr const char* red          = "\033[31m";
    constexpr const char* green        = "\033[32m";
    constexpr const char* yellow       = "\033[33m";
    constexpr const char* blue         = "\033[34m";
    constexpr const char* magenta      = "\033[35m";
    constexpr const char* cyan         = "\033[36m";
    constexpr const char* white        = "\033[37m";
    constexpr const char* brightYellow = "\033[93m";
    constexpr const char* brightMagenta= "\033[95m";
    constexpr const char* reset        = "\033[0m";
}

// Return an ANSI color code based on the byte offset.
const char* getColorForOffset(size_t offset) {
    if (offset < 4)                         return colors::red;             // magic[4]
    else if (offset == 4)                   return colors::green;           // format_major
    else if (offset == 5)                   return colors::yellow;          // format_minor
    else if (offset == 6)                   return colors::blue;            // configs (1 bytes)
    else if (offset == 7)                   return colors::magenta;         // endianess/res0 (1 byte)
    else if (offset >= 8 && offset <= 11)   return colors::cyan;            // bitfields for sizes (4 bytes)
    else if (offset >= 12 && offset <= 13)  return colors::white;           // docs_count (2 bytes)
    else if (offset >= 14 && offset <= 15)  return colors::brightYellow;    // res[2]
    else if (offset >= 16 && offset <= 23)  return colors::brightMagenta;   // offset_symbols (8 bytes)
    else                                    return colors::reset;           // sections & others
}

void printSpan(std::span<const uint8_t> data) {
    constexpr size_t groupSize = 32; // We'll print 32 bytes per line

    for (size_t offset = 0; offset < data.size(); offset += groupSize) {
        // Print the offset (address) header
        std::print("{:04x}: ", offset);

        // Determine how many bytes are in this group.
        size_t bytesInGroup = std::min(groupSize, data.size() - offset);

        // Print hexadecimal bytes with color coding.
        for (size_t i = 0; i < groupSize; ++i) {
            if (i < bytesInGroup) {
                size_t byte_offset = offset + i;
                const char* color = getColorForOffset(byte_offset);
                // Print the colored two-digit hex and then reset color after each byte.
                std::print("{}{:02x}{} ", color, data[byte_offset], colors::reset);
            } else {
                // Add spacing if no data.
                std::print("   ");
            }
        }

        // Print ASCII representation
        std::print(" | ");
        for (size_t i = 0; i < bytesInGroup; ++i) {
            uint8_t byte = data[offset + i];
            char ch = std::isprint(byte) ? static_cast<char>(byte) : '.';
            // Use same color as the hex part.
            const char* color = getColorForOffset(offset + i);
            std::print("{}{}{}", color, ch, colors::reset);
        }
        std::print("\n");
    }
}


/*
void printSpan(std::span<const uint8_t> data) {
    constexpr size_t groupSize = 32; // Print 32 bytes per group

    for (size_t offset = 0; offset < data.size(); offset += groupSize) {
        // Print the offset for the current line.
        std::print("{:04x}: ", offset);

        // Determine how many bytes are in this group.
        size_t bytesInGroup = std::min(groupSize, data.size() - offset);

        // Print the hexadecimal values for this group without the "0x" prefix.
        for (size_t i = 0; i < groupSize; ++i) {
            if (i < bytesInGroup) {
                // Print two-digit hex
                std::print("{:02x} ", data[offset + i]);
            } else {
                // Maintain spacing for alignment when group is incomplete.
                std::print("   ");
            }
        }

        // Append a separator before printing the ASCII characters.
        std::print(" | ");

        // Print the ASCII representation.
        for (size_t i = 0; i < bytesInGroup; ++i) {
            uint8_t byte = data[offset + i];
            // Use std::isprint to check if the byte is a printable ASCII character.
            if (std::isprint(byte)) {
                std::print("{}", static_cast<char>(byte));
            } else {
                std::print(".");
            }
        }

        // End the line.
        std::print("\n");
    }
}
*/

int main(){
    DocBuilder<{.symbols=xml::builder_config_t::COMPRESS_ALL,.raw_strings=true}> bld;

    //Optionally, you can reserve space for the containers used by the builder. This way you can cut down on allocations.
    bld.reserve(1024*4,1024*4);

    constexpr std::string_view texts[] = {"Text 0", "Text 1", "Text 2"};

    std::vector<std::pair<sv,std::vector<uint8_t>>> fragments;
    fragments.reserve(sizeof(texts)/sizeof(std::string_view));


    //Here we constructs multiple documents but we preserve the table of symbols.
    //Please notice the usage of `close_frame` and the final `extract_symbols` in place of the "normal" `close`.

    std::print("Constructing frames.\n");

    for(auto text : texts){
        std::print("Building {}\n",text);
        bld.xml();
        bld.begin("tag-root");
            bld.attr("doc","document-value");
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
    auto symbols = bld.extract_symbols();
    if(!symbols.has_value()){
        std::print(std::cerr,"Unable to complete construction of the tree; failure to handle symbols.");
        exit(1);
    }


    //We now serialize the archive after creation onto a stream, just for demonstrative purposes.
    std::print("Creating archive.\n");
    std::stringstream memstream;

    stored::Archive archive({.symbols=xml::builder_config_t::COMPRESS_ALL,.raw_strings=true},std::move(fragments),std::move(*symbols));

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
    printSpan(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(tmp_str.data()), tmp_str.size()));

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