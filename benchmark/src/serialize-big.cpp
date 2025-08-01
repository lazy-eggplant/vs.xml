#include <filesystem>
#include <iostream>
#include <ostream>
#include <print>
#include <sstream>

#include <string_view>
#include <vs-xml/commons.hpp>
#include <vs-xml/parser.hpp>
#include <vs-xml/serializer.hpp>
#include <vs-xml/document-builder.hpp>

#include <mio/mmap.hpp>
#include <pugixml.hpp>


int test_vs(std::string_view xmlInput){
    try{
        VS_XML_NS::DocumentBuilder<{.symbols=VS_XML_NS::builder_config_t::EXTERN_REL,.raw_strings=true}> bld(xmlInput);
        VS_XML_NS::Parser parser(xmlInput, bld);
        std::ignore = parser.parse();

        auto tree = bld.close();
        if(!tree.has_value()){
            std::cerr << "Error while closing the document " << (int)tree.error() << "\n";
            return 3;
        }

        std::string str;
        std::stringstream file(str);

        tree->print_fast(file);

    }catch (const std::exception &ex) {
        std::cerr << "Error while testing: " << ex.what() << "\n";
        return 2;
    }
    return 0;
}


int test_vs2(std::string_view binInput){
    try{
        mio::mmap_source mmap("./assets/nasa_10_f_bs.xml.bin");
        std::span<const uint8_t> binInput((const uint8_t*)mmap.data(),mmap.size());

        
        auto tree =VS_XML_NS::Document::from_binary(binInput);

        std::string str;
        std::stringstream file(str);

        //tree.print(file);
        tree->print_fast(file);

    }catch (const std::exception &ex) {
        std::cerr << "Error while testing: " << ex.what() << "\n";
        return 2;
    }
    return 0;
}


int test_pugi(std::string_view xmlInput){
    try{
        mio::mmap_source mmap("./assets/nasa_10_f_bs.xml");
        std::string_view xmlInput(mmap.data(),mmap.size());

        
        pugi::xml_document document;
        document.load_buffer(xmlInput.data(), xmlInput.length());

        std::string str;
        std::stringstream file(str);

        document.save(file);

    }catch (const std::exception &ex) {
        std::cerr << "Error while testing: " << ex.what() << "\n";
        return 2;
    }
    return 0;
}


int main(int argc, const char* argv[]) {
    mio::mmap_source mmap1("./assets/nasa_10_f_bs.xml");
    std::string_view xmlInput1(mmap1.data(),mmap1.size());

    mio::mmap_source mmap2("./assets/nasa_10_f_bs.xml");
    std::string_view xmlInput2(mmap2.data(),mmap2.size());

    mio::mmap_source mmap3("./assets/nasa_10_f_bs.xml.bin");
    std::string_view binInput(mmap3.data(),mmap3.size());
    for(int i = 0; i<3; i++){
        std::vector<decltype(std::chrono::system_clock::now())> ticks;
        ticks.push_back(std::chrono::system_clock::now());
        test_vs(xmlInput1);
        ticks.push_back(std::chrono::system_clock::now());
        test_vs2(binInput);
        ticks.push_back(std::chrono::system_clock::now());
        test_pugi(xmlInput2);
        ticks.push_back(std::chrono::system_clock::now());
        
        std::print("vs   :   {}\n",  (ticks[1]-ticks[0]).count());
        std::print("vs2  :   {}\n",  (ticks[2]-ticks[1]).count());
        std::print("pugi :   {}\n",  (ticks[3]-ticks[2]).count());
    }
    

}