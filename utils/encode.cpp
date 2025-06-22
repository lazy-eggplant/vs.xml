#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <print>

#include <vs-xml/commons.hpp>
#include <vs-xml/parser.hpp>
#include <vs-xml/serializer.hpp>
#include <vs-xml/document-builder.hpp>

#include <mio/mmap.hpp>

template<xml::builder_config_t cfg>
int encode(std::filesystem::path input, std::filesystem::path output){
    try{
        mio::mmap_source mmap(input.c_str());
        std::string_view xmlInput(mmap.data(),mmap.size());

        xml::DocumentBuilder<cfg> bld;
        xml::Parser parser(xmlInput, bld);
        if(auto ret = parser.parse(); !ret.has_value())throw std::runtime_error(std::string(ret.error().msg()));

        auto tree = bld.close();
        if(!tree.has_value()){
            std::cerr << "Error while closing the document " << (int)tree.error() << "\n";
            return 3;
        }

        //tree->print(std::cout);

        std::ofstream file(output,std::ios::binary|std::ios::out);
        if(!file.is_open()){
            std::cerr << "Error opening file\n";
            return 4;
        }

        if(!tree->save_binary(file)){
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
    return encode<{.symbols=xml::builder_config_t::COMPRESS_ALL,.raw_strings=true}>(argv[1],argv[2]);
}