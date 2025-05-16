//TODO: Adapt in a simple form, removing mio and using stream for the example.

#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <print>

#include <vs-xml/commons.hpp>
#include <vs-xml/parser.hpp>
#include <vs-xml/serializer.hpp>
#include <vs-xml/document.hpp>

#include <mio/mmap.hpp>

int decode(std::filesystem::path input, std::filesystem::path output){
    try{
        mio::mmap_source mmap(input.c_str());
        std::string_view binaryInput(mmap.data(),mmap.size());
        auto tree = xml::Document::from_binary(binaryInput);

        std::ofstream file(output,std::ios::binary|std::ios::out);
        if(!file.is_open()){
            std::cerr << "Error opening file\n";
            return 4;
        }

        if(!tree.print(file)){
            std::cerr << "Error in serialization to XML\n";
            return 5;
        }
    }catch (const std::exception &ex) {
        std::cerr << "Error while parsing XML: " << ex.what() << "\n";
        return 2;
    }

    return 0;
}

int main(int argc, const char* argv[]) {
    if(argc<3){std::cerr<<"Wrong usage, pass input file and output file as args.";return 1;}
    return decode(argv[1],argv[2]);
}