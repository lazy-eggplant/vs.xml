#pragma once

/**
 * @file document-builder.hpp
 * @author karurochari
 * @brief Document builder wrapper implementation for tree builder
 * @date 2025-06-20
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include  <expected>

#include  <vs-xml/commons.hpp>
#include  <vs-xml/document.hpp>
#include  <vs-xml/tree-builder.hpp>

namespace VS_XML_NS{

/**
 * @brief Specialized builder to construct a document.
*/
template<builder_config_t cfg = {}>
struct DocBuilder : TreeBuilder<cfg>{
    protected:
        using TreeBuilder<cfg>::close;
        
    public:
    constexpr static inline builder_config_t configs = cfg;
    constexpr static inline bool is_document = true;

    DocBuilder(auto ... a):TreeBuilder<cfg>(a...){
        this->begin("ROOT");
    }

    inline details::BuilderBase::error_t xml(){
        return details::BuilderBase::proc(TreeBuilder<cfg>::rsv( TreeBuilder<cfg>::label("xml version=\"1.0\" encoding=\"UTF-8\"")));
    }

    [[nodiscard]] inline std::expected<stored::Document,details::BuilderBase::error_t> close(){
        this->end();
        details::BuilderBase::close();
        if constexpr (
            cfg.symbols==builder_config_t::symbols_t::COMPRESS_ALL ||
            cfg.symbols==builder_config_t::symbols_t::COMPRESS_LABELS ||
            cfg.symbols==builder_config_t::symbols_t::OWNED 
        )return stored::Document(configs,std::exchange(this->buffer,{}),std::exchange(this->symbols.symbols,{}));
        else return stored::Document(configs,std::exchange(this->buffer,{}),this->symbols.symbols.data());
    }

    [[nodiscard]] std::expected<binary_header_t::section_t,details::BuilderBase::error_t> close_frame(std::string_view name=""){
        this->end();
        auto tmp = TreeBuilder<configs>::close_frame(name);
        this->begin("ROOT");
        return tmp;
    }
    
    [[nodiscard]] std::optional<std::pair<std::vector<uint8_t>,std::vector<uint8_t>>> extract(){
        this->end();
        details::BuilderBase::close();
        return TreeBuilder<configs>::extract();
    }
};



}