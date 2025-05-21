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

#include <ranges>

#include  <vs-xml/commons.hpp>
#include  <vs-xml/document.hpp>

/*
 ROOT
    INDEX
        PAIR(hash, addr)
    DOCUMENTS
        DOCUMENT(name)
            ?XML
            ...
        DOCUMENT(name)
            ?XML
            ...
*/

namespace VS_XML_NS{

//TODO: to fully rework after the symbol handling is split from the builder.
template<builder_config_t cfg = {}>
struct BatchBuilder : DocBuilder<cfg>{
    protected:
    std::vector<std::expected<Document,details::BuilderBase::error_t>> documents;

    public:

    inline std::expected<void,details::BuilderBase::error_t> close(){
        auto ret = details::BuilderImpl<cfg.symbols>::close();

        documents.emplace_back(DocBuilder<cfg>::close());
        this->buffer.clear();   //TODO: Check if it must be resized.
        this->open=true;
        this->attribute_block=false;

        return ret;
    }

    
    [[nodiscard]] inline std::vector<std::expected<Document,details::BuilderBase::error_t>> close_all(){
        static_assert(cfg.symbols==builder_config_t::symbols_t::COMPRESS_ALL ||
            cfg.symbols==builder_config_t::symbols_t::COMPRESS_LABELS ||
            cfg.symbols==builder_config_t::symbols_t::OWNED );

        //Apply the latest symbol table to all of them before closing.
        for(auto& doc: documents){
            doc->symbols = this->symbols;
        }

        return std::move(documents);
    }

    //TODO:
    bool save_binary(std::ostream& out)const;

    //TODO:
    [[nodiscard]] static std::vector<std::expected<Document, TreeRaw::from_binary_error_t>> from_binary(std::span<uint8_t> region);
    [[nodiscard]] static std::vector<std::expected<const TreeRaw , TreeRaw::from_binary_error_t>> from_binary(std::string_view region);
};

}