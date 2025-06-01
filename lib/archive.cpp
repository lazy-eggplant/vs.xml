#include <expected>
#include <vs-xml/archive.hpp>

namespace VS_XML_NS{



bool ArchiveRaw::save_binary(std::ostream& out)const{
    if(configs.symbols==builder_config_t::EXTERN_ABS)return false; //Symbols not relocatable.

    binary_header_t header{};
    header.configs = configs;
    if(header.configs.symbols==builder_config_t::EXTERN_REL)header.configs.symbols=builder_config_t::OWNED; //Symbols are copied even if the where shared, so they are now owned.

    size_t align_symbols = (symbols.size_bytes()%16==0)?0:(16-symbols.size_bytes()%16);
    header.offset_symbols = header.size();
    header.docs_count = documents.size();

    out.write((const char*)&header, sizeof(header));

    size_t current=0;
    for(auto& document: this->documents){
        binary_header_t::section_t section = {
            {document.first.base,document.first.length},
            header.size()+symbols.size_bytes()+align_symbols+current,
            header.size()+symbols.size_bytes()+align_symbols+current+document.second.size()
        };
        out.write((const char*)&section, sizeof(binary_header_t::section_t));
        current+=document.second.size();
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


std::expected<ArchiveRaw, ArchiveRaw::from_binary_error_t> ArchiveRaw::from_binary(std::span<uint8_t> region){
    std::vector<std::pair<sv,std::span<uint8_t>>> documents;
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

    documents.reserve(header.docs_count);

    for(size_t i=0;i<header.docs_count;i++){
        auto& section = header.sections[i];
        documents.emplace_back(
            sv{section.name.base,section.name.length},
            std::span<uint8_t>{region.data()+header.region(i).start, region.data()+header.region(i).end}
        );
    }

    symbols=std::span<uint8_t>{region.data()+header.size(), region.data()+header.offset_symbols};

    return ArchiveRaw(header.configs,std::move(documents),symbols);
}



std::expected<ArchiveRaw, ArchiveRaw::from_binary_error_t> ArchiveRaw::from_binary(std::string_view region){
    return from_binary(std::span<uint8_t>{(uint8_t*)region.begin(),(uint8_t*)region.end()});
}


}