#include "vs-xml/commons.hpp"
#include "vs-xml/utils/warn-suppress.h"
#include <expected>
#include <vs-xml/archive.hpp>

namespace VS_XML_NS{



bool ArchiveRaw::save_binary(std::ostream& out)const{
    if(configs.symbols==builder_config_t::EXTERN_ABS)return false; //Symbols not relocatable.

    binary_header_t header{};
    header.configs = configs;
    if(header.configs.symbols==builder_config_t::EXTERN_REL)header.configs.symbols=builder_config_t::OWNED; //Symbols are copied even if the where shared, so they are now owned.

    size_t align_symbols = (header.size()+symbols.size_bytes()%16==0)?0:(16-(header.size()+symbols.size_bytes())%16);
    header.length_of_symbols = symbols.size_bytes();
    header.docs_count = index.size();

    out.write((const char*)&header, sizeof(header));

    delta_ptr_t current=0;
    for(auto& document: this->index){
        binary_header_t::section_t section = {
            {document.name.base,document.name.length},
            current,
            document.length
        };
        out.write((const char*)&section, sizeof(binary_header_t::section_t));
        current+=document.length;
    }

    out.write((const char*)symbols.data(), symbols.size_bytes());

    if(align_symbols!=0){
        //std::printf("----Align %d %d\n", align_symbols,symbols.size_bytes());
        char tmp[16]{};
        out.write(tmp, align_symbols);
    }

    for(auto& document: this->index){
        out.write((const char*)this->buffer.data()+document.base, document.length);
    }
    
    out.flush();
    return true;
}


std::expected<ArchiveRaw, ArchiveRaw::from_binary_error_t> ArchiveRaw::from_binary(std::span<uint8_t> region){
    std::span<uint8_t> symbols;

    const binary_header_t& header = *(const binary_header_t*)region.data();

    if(region.size_bytes() < header.size())
        return std::unexpected(from_binary_error_t{from_binary_error_t::HeaderTooSmall});
        
    if(std::memcmp(header.magic, "$XML", 4) != 0)
        return std::unexpected(from_binary_error_t{from_binary_error_t::MagicMismatch});
    
    if(header.format_major != format_major)
        return std::unexpected(from_binary_error_t{from_binary_error_t::MajorVersionMismatch});
    
    if(header.format_minor > format_minor)
        return std::unexpected(from_binary_error_t{from_binary_error_t::MinorVersionTooHigh});

    if  (
            header.size__delta_ptr!=sizeof(delta_ptr_t) || 
            header.size__xml_count!=sizeof(xml_count_t) ||
            header.size__xml_enum_size!=sizeof(xml_enum_size_t) ||
            header.size__xml_size!=sizeof(xml_size_t)
        ) return std::unexpected(from_binary_error_t{from_binary_error_t::TypeMismatch});

    auto endianess = std::endian::native==std::endian::little?binary_header_t::endianess_t::LITTLE:binary_header_t::endianess_t::BIG;
    if(header.endianess!=endianess) return std::unexpected(from_binary_error_t{from_binary_error_t::TypeMismatch});

    symbols=std::span<uint8_t>{region.data()+header.size(), header.length_of_symbols};

    WARN_PUSH;
    WARN_IGNORE("-Waddress-of-packed-member");
    //`sections` alignment is safe since as it is being guarded by a separate static_assert to be 64bit aligned.
    return ArchiveRaw(header.configs,{header.sections,header.docs_count},{region.data()+header.start_data(),region.data()+region.size_bytes()},symbols);
    WARN_POP;
}



std::expected<const ArchiveRaw, ArchiveRaw::from_binary_error_t> ArchiveRaw::from_binary(std::span<const uint8_t> region){
    return from_binary(std::span<uint8_t>{(uint8_t*)region.data(),(uint8_t*)region.data()+region.size_bytes()});
}


}