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

#include  <vs-xml/commons.hpp>
#include  <vs-xml/tree.hpp>
#include  <vs-xml/builder.hpp>

namespace VS_XML_NS{

//TODO: Not fully implemented yet, no wrapping or checks which are needed to store a valid XML.

template<builder_config_t cfg>
struct DocBuilder;

struct DocumentRaw : TreeRaw {
    using TreeRaw::TreeRaw;

    //TODO: add real root opposed to document root.
    inline bool print(std::ostream& out, const print_cfg_t& cfg = {})const{
        for(auto& it: TreeRaw::root().children()){
            if(!print_h(out, cfg, &it))return false;
        }
        return true;
    }

    /**
     * @brief Return the root of the proper tree inside the document (if present)
     * 
     * @return std::optional<node_iterator> 
     */
    inline std::optional<node_iterator> tree_root() const{
        auto c  = TreeRaw::root().children();
        auto it = std::ranges::find_if(c,[](auto e){return e.type()==type_t::ELEMENT;});
        if(it!=c.end()) return it;
        return {};
    }

    static inline DocumentRaw from_binary(std::span<uint8_t> region){return DocumentRaw(TreeRaw::from_binary(region));}
    static inline const DocumentRaw from_binary(std::string_view region){return DocumentRaw(TreeRaw::from_binary(region));}

    template<builder_config_t cfg>
    friend struct DocBuilder;

    protected:
        DocumentRaw(TreeRaw&& src):TreeRaw(src){}
        DocumentRaw(const TreeRaw&& src):TreeRaw(src){}

};

struct Document : DocumentRaw {
    private:
    using DocumentRaw::rsv;
    using DocumentRaw::clone;
    using DocumentRaw::root;


    public:
    inline Document(DocumentRaw&& ref):DocumentRaw(std::move(ref)){}
    inline Document(const DocumentRaw&& ref):DocumentRaw(std::move(ref)){}

    inline const Tree slice(const element_t* ref=nullptr) const{return DocumentRaw::slice(ref);}
    inline Tree clone(const element_t* ref=nullptr, bool reduce=true) const{return DocumentRaw::clone(ref,reduce);}

    inline  wrp::base_t<element_t>  root() {return wrp::base_t<element_t>{*(const TreeRaw*)this, &TreeRaw::root()};}

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