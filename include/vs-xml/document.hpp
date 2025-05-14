#pragma once

/**
 * @file document.hpp
 * @author karurochari
 * @brief Document wrapper for a tree
 * @date 2025-05-09
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "commons.hpp"
#include "tree.hpp"
#include "builder.hpp"

namespace VS_XML_NS{

//TODO: Not fully implemented yet, no wrapping or checks which are needed to store a valid XML.

template<builder_config_t cfg>
struct DocBuilder;

struct DocumentRaw : TreeRaw {
    using TreeRaw::TreeRaw;

    //TODO: add real root opposed to document root.
    inline bool print(std::ostream& out, const print_cfg_t& cfg = {})const{
        for(auto& it: root().children()){
            if(!print_h(out, cfg, &it))return false;
        }
        return true;
    }

    template<builder_config_t cfg>
    friend struct DocBuilder;
};

struct Document : DocumentRaw {
    public:
    inline Document(DocumentRaw&& ref):DocumentRaw(std::move(ref)){}
    inline Document(const DocumentRaw&& ref):DocumentRaw(std::move(ref)){}

    inline const Tree slice(const element_t* ref=nullptr) const{return DocumentRaw::slice(ref);}
    inline Tree clone(const element_t* ref=nullptr, bool reduce=true) const{return DocumentRaw::clone(ref,reduce);}

    wrp::base_t<element_t> root() const;

    inline DocumentRaw& downgrade(){return *this;}
};

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
        return details::BuilderBase::proc(TreeBuilder<cfg>::rsv( TreeBuilder<cfg>::symbol("xml version=\"1.0\" encoding=\"UTF-8\"")));
    }

    inline std::expected<Document,error_t> close(){
        this->end();
        details::BuilderImpl<cfg.symbols>::close();
        if constexpr (
            cfg.symbols==builder_config_t::symbols_t::COMPRESS_ALL ||
            cfg.symbols==builder_config_t::symbols_t::COMPRESS_LABELS ||
            cfg.symbols==builder_config_t::symbols_t::OWNED 
        )return Document(DocumentRaw(configs,std::move(this->get_buffer()),std::move(this->symbols_i)));
        else return Document(DocumentRaw(configs,std::move(this->get_buffer()),this->symbols.data()));
    }
    
};


}