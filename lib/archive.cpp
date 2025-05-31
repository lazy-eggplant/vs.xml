#include <vs-xml/archive.hpp>

namespace VS_XML_NS{



bool ArchiveRaw::save_binary(std::ostream& out)const{
    if(configs.symbols==builder_config_t::EXTERN_ABS)return false; //Symbols not relocatable.

    binary_header_t header{};
    header.configs = configs;
    if(header.configs.symbols==builder_config_t::EXTERN_REL)header.configs.symbols=builder_config_t::OWNED; //Symbols are copied even if the where shared, so they are now owned.

    size_t align_symbols = (symbols.size_bytes()%16==0)?0:(16-symbols.size_bytes()%16);
    header.offset_symbols = header.size();

    out.write((const char*)&header, sizeof(header));

    for(auto& document: this->documents){
        binary_header_t::section_t section = {
            {document.first.base,document.first.length},
            header.size()+symbols.size_bytes()+align_symbols,
            header.size()+symbols.size_bytes()+align_symbols+document.second.size()
        };
        out.write((const char*)&section, sizeof(binary_header_t::section_t));
    }

    out.write((const char*)symbols.data(), symbols.size_bytes());

    if(align_symbols!=0){
        char tmp[16]{};
        out.write(tmp, align_symbols);
    }

    for(auto& document: this->documents){
        out.write((const char*)document.second.data(), document.second.size());
    }
    
    out.flush();
    return true;
}

/*
std::expected<ArchiveRaw, ArchiveRaw::from_binary_error_t> ArchiveRaw::from_binary(std::span<uint8_t> region){
   const binary_header_t& header = *(const binary_header_t*)region.data();

    if(region.size_bytes() < header.size())
        return std::unexpected(from_binary_error_t{from_binary_error_t::HeaderTooSmall});
        
    if(std::memcmp(header.magic, "$XML", 4) != 0)
        return std::unexpected(from_binary_error_t{from_binary_error_t::MagicMismatch});
    
    if(header.format_major != format_major)
        return std::unexpected(from_binary_error_t{from_binary_error_t::MajorVersionMismatch});
    
    if(header.format_minor > format_minor)
        return std::unexpected(from_binary_error_t{from_binary_error_t::MinorVersionTooHigh});
    
    if(header.docs_count != 1)
        return std::unexpected(from_binary_error_t{from_binary_error_t::TooManyDocs});

    if  (
            header.size__delta_ptr!=sizeof(delta_ptr_t) || 
            header.size__xml_count!=sizeof(xml_count_t) ||
            header.size__xml_enum_size!=sizeof(xml_enum_size_t) ||
            header.size__xml_size!=sizeof(xml_size_t)
        ) return std::unexpected(from_binary_error_t{from_binary_error_t::TypeMismatch});

    auto endianess = std::endian::native==std::endian::little?binary_header_t::endianess_t::LITTLE:binary_header_t::endianess_t::BIG;
    if(header.endianess!=endianess) return std::unexpected(from_binary_error_t{from_binary_error_t::TypeMismatch});

    //TODO: Restore bounds checks (?) on sections?

    return TreeRaw(header.configs,
        std::span<uint8_t>{region.data()+header.region(0).start, region.data()+header.region(0).end},
        std::span<uint8_t>{region.data()+header.offset_symbols, region.data()+header.region(0).start}
    );
}
*/


std::expected<const TreeRaw, TreeRaw::from_binary_error_t>  TreeRaw::from_binary(std::string_view region){
    return from_binary(std::span<uint8_t>{(uint8_t*)region.begin(),(uint8_t*)region.end()});
}


}