#include "vs-xml/commons.hpp"
#include "vs-xml/utils/warn-suppress.h"
#include <expected>
#include <vs-xml/archive.hpp>
#include <cstring>

namespace VS_XML_NS{

bool ArchiveRaw::save_binary(std::ostream& out)const{
    if(configs.symbols==builder_config_t::EXTERN_ABS)return false; //Symbols not relocatable.

    binary_header_t header{};
    header.configs = configs;
    if(header.configs.symbols==builder_config_t::EXTERN_REL)header.configs.symbols=builder_config_t::OWNED; //Symbols are copied even if the where shared, so they are now owned.

    size_t align_symbols = 0;
    header.length_of_symbols = symbols.size_bytes();
    header.docs_count = index.size();
    {
        const size_t total = header.size() + symbols.size_bytes();
        align_symbols = (total%16==0)?0:(16-total%16);
    }

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

    // Never dereference the header before proving the region can hold it.
    if(region.size_bytes() < sizeof(binary_header_t))
        return std::unexpected(from_binary_error_t{from_binary_error_t::HeaderTooSmall});

    const binary_header_t& header = *(const binary_header_t*)region.data();

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

    const size_t region_size = region.size_bytes();
    if(header.size() > region_size)
        return std::unexpected(from_binary_error_t{from_binary_error_t::HeaderTooSmall});

    const size_t after_header = region_size - header.size();
    if(header.length_of_symbols > after_header)
        return std::unexpected(from_binary_error_t{from_binary_error_t::SymbolsOutOfBounds});

    const size_t data_start = header.start_data();
    if(data_start > region_size)
        return std::unexpected(from_binary_error_t{from_binary_error_t::TruncatedSpan});

    // In-place node structs are reinterpreted from this address, so it must be aligned.
    if(reinterpret_cast<uintptr_t>(region.data()+data_start) % alignof(std::max_align_t) != 0)
        return std::unexpected(from_binary_error_t{from_binary_error_t::MisalignedRegion});

    const size_t data_size = region_size - data_start;

    // Every section must lie fully inside the data region.
    for(size_t i=0;i<header.docs_count;i++){
        const binary_header_t::section_t sec = header.region(i);
        if(sec.base < 0 || (size_t)sec.base > data_size)
            return std::unexpected(from_binary_error_t{from_binary_error_t::TreeOutOfBounds});
        if(sec.length > data_size - (size_t)sec.base)
            return std::unexpected(from_binary_error_t{from_binary_error_t::TreeOutOfBounds});
    }

    symbols=std::span<uint8_t>{region.data()+header.size(), (size_t)header.length_of_symbols};

    WARN_PUSH;
    WARN_IGNORE("-Waddress-of-packed-member");
    //`sections` alignment is safe since as it is being guarded by a separate static_assert to be 64bit aligned.
    return ArchiveRaw(header.configs,{header.sections,header.docs_count},{region.data()+data_start,data_size},symbols);
    WARN_POP;
}

std::expected<const ArchiveRaw, ArchiveRaw::from_binary_error_t> ArchiveRaw::from_binary(std::span<const uint8_t> region){
    return from_binary(std::span<uint8_t>{(uint8_t*)region.data(),(uint8_t*)region.data()+region.size_bytes()});
}

}
