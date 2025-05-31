#pragma once

/**
 * @file folder.hpp
 * @author karurochari
 * @brief A collection of multiple documents, stored as a single tree. They use shared symbols and a common address space.
 * @date 2025-05-21
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <string_view>
#include  <vs-xml/commons.hpp>
#include  <vs-xml/document.hpp>

namespace VS_XML_NS{

/**
 * @brief Base class for an archive of documents.
 * @warning Unless you need MAXIMUM PERFORMANCE, your are better using its derived VS_XML_NS::Archive
 * @details This one does not wrap returned XML entities nor string views, so you are left on your own to handle them, but you might gain few points in complex pipelines.
 */
struct ArchiveRaw{
    private:
        std::vector<std::pair<sv,std::vector<uint8_t>>> documents;
        std::vector<uint8_t> symbols_i;
        std::span<uint8_t> symbols;
        builder_config_t configs;

    public:

    using from_binary_error_t = TreeRaw::from_binary_error_t;

    bool save_binary(std::ostream& out)const;

    [[nodiscard]] static std::expected<ArchiveRaw, ArchiveRaw::from_binary_error_t> from_binary(std::span<uint8_t> region);
    [[nodiscard]] static std::expected<const ArchiveRaw , ArchiveRaw::from_binary_error_t> from_binary(std::string_view region);

    inline std::string_view rsv(sv s) const{
        return std::string_view(s.base+(char*)symbols.data(),s.base+(char*)symbols.data()+s.length);
    }

    inline std::optional<DocumentRaw> get(size_t idx){
        //xml_assert(documents.size()>idx, "Out of bounds document selected");
        if(idx>documents.size())return {};
        auto v = documents.at(idx);
        return DocumentRaw(configs,std::span{v.second.begin(),v.second.end()},std::span{symbols.begin(),symbols.end()});
    }
};

/**
 * @brief the archive class you should use.
 */
struct Archive : ArchiveRaw{
    inline std::optional<DocumentRaw> get(size_t idx){
        auto tmp = ArchiveRaw::get(idx);
        if(tmp.has_value())return Document(std::move(*tmp));
        else return {};
    }
};

}