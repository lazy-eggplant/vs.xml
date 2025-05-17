#include <filesystem>
#include <iostream>
#include <ostream>
#include <print>
#include <sstream>

#include <vs-xml/commons.hpp>
#include <vs-xml/parser.hpp>
#include <vs-xml/serializer.hpp>
#include <vs-xml/document.hpp>

#include <mio/mmap.hpp>
#include <pugixml.hpp>


int test_vs(){
    try{
        mio::mmap_source mmap("./assets/nasa_10_f_bs.xml");
        std::string_view xmlInput(mmap.data(),mmap.size());

        
        xml::DocBuilder<{.symbols=xml::builder_config_t::EXTERN_REL,.raw_strings=true}> bld(xmlInput);
        xml::Parser parser(xmlInput, bld);
        parser.parse();

        auto tree = bld.close();
        if(!tree.has_value()){
            std::cerr << "Error while closing the document " << tree.error() << "\n";
            return 3;
        }

        std::string str;
        std::stringstream file(str);

        //tree->print(file);

    }catch (const std::exception &ex) {
        std::cerr << "Error while testing: " << ex.what() << "\n";
        return 2;
    }
    return 0;
}


int test_vs2(){
    try{
        mio::mmap_source mmap("./assets/nasa_10_f_bs.xml.bin");
        std::string_view binInput(mmap.data(),mmap.size());

        
        auto tree =xml::Document::from_binary(binInput);

        std::string str;
        std::stringstream file(str);

        //tree.print(file);
        tree.save_binary(file);

    }catch (const std::exception &ex) {
        std::cerr << "Error while testing: " << ex.what() << "\n";
        return 2;
    }
    return 0;
}


int test_pugi(){
    try{
        mio::mmap_source mmap("./assets/nasa_10_f_bs.xml");
        std::string_view xmlInput(mmap.data(),mmap.size());

        
        pugi::xml_document document;
        document.load_buffer(xmlInput.data(), xmlInput.length());

        std::string str;
        std::stringstream file(str);

        //document.save(file);

    }catch (const std::exception &ex) {
        std::cerr << "Error while testing: " << ex.what() << "\n";
        return 2;
    }
    return 0;
}


int main(int argc, const char* argv[]) {
    for(int i = 0; i<2; i++){
        auto t0 = std::chrono::system_clock::now();
        test_vs();
        auto t1 = std::chrono::system_clock::now();
        test_pugi();
        auto t2 = std::chrono::system_clock::now();
        test_vs2();
        auto t3 = std::chrono::system_clock::now();

        std::print("vs:   {}\nvs2:  {}\npugi: {}\n",(t1-t0).count(), (t3-t2).count(), (t2-t1).count());
    }
    

}