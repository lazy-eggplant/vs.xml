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

#include  <expected>
#include  <vs-xml/commons.hpp>
#include  <vs-xml/tree.hpp>
#include  <vs-xml/tree-builder.hpp>

namespace VS_XML_NS{

//TODO: Implemented but very lass on constraints. No wrapping nor checks which are needed to store a valid XML.

template<builder_config_t cfg>
struct DocumentBuilder;

/**
 * @brief Base class for an XML document. 
 * @warning Unless you need MAXIMUM PERFORMANCE, your are better using its derived VS_XML_NS::Document
 * @details This one does not wrap returned XML entities nor string views, so you are left on your own to handle them, but you might gain few points in complex pipelines.
 * 
 */
struct DocumentRaw : TreeRaw {
    using TreeRaw::TreeRaw;

    inline bool print(std::ostream& out, const print_cfg_t& cfg = {})const{
        for(auto& it: TreeRaw::root().children()){
            if(!TreeRaw::print(out, cfg, &it))return false;
        }
        return true;
    }

    /**
     * @brief Return the root of the proper tree inside the document (if present)
     * 
     * @return std::optional<node_iterator> 
     */
    [[nodiscard]] inline std::optional<node_iterator> tree_root() const{
        auto c  = TreeRaw::root().children();
        auto it = std::ranges::find_if(c,[](auto e)static{return e.type()==type_t::ELEMENT;});
        if(it!=c.end()) return it;
        return {};
    }

    [[nodiscard]] static inline std::expected<DocumentRaw,TreeRaw::from_binary_error_t> from_binary(std::span<uint8_t> region){
        std::expected<TreeRaw, TreeRaw::from_binary_error_t> t = TreeRaw::from_binary(region); 
        if(!t.has_value())return std::unexpected(t.error()); 
        else return DocumentRaw(std::move(*t));
    }
    [[nodiscard]] static inline const std::expected<const DocumentRaw,TreeRaw::from_binary_error_t> from_binary(std::span<const uint8_t> region){
        std::expected<const TreeRaw, TreeRaw::from_binary_error_t> t = TreeRaw::from_binary(region); 
        if(!t.has_value())return std::unexpected(t.error()); 
        else return DocumentRaw(std::move(*t));
    }

    template<builder_config_t cfg>
    friend struct DocumentBuilder;

    //TODO: Replace with proper prototypes, and incapsulate the mv mechanism away as it is an implementation detail, not semantically correct.
    DocumentRaw(TreeRaw&& src):TreeRaw(src){}
    DocumentRaw(const TreeRaw&& src):TreeRaw(src){}

};

/**
 * @brief The suggested-to-use Document class.
 * @details In general, not to be used directly. Either load a binary file or generate it via a builder.
 */
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

    inline  wrp::base_t<unknown_t>  root() {return wrp::base_t<unknown_t>{*(const TreeRaw*)this, &TreeRaw::root()};}

    ///Cast this document as a raw document
    inline DocumentRaw& downgrade(){return *this;}

    ///Cast this const document as a const raw tree
    inline const DocumentRaw& downgrade() const{return *this;}
};


template<>
struct StorageFor<DocumentRaw>{
    std::vector<uint8_t> buffer_i;
    std::vector<uint8_t> symbols_i;

    StorageFor(const builder_config_t& cfg, std::vector<uint8_t>&& buf, std::vector<uint8_t>&& sym):buffer_i(buf),symbols_i(sym){}
    StorageFor(const builder_config_t& cfg, std::vector<uint8_t>&& buf, const void* label_offset=nullptr):buffer_i(buf){}

    static TreeRaw bind(const StorageFor& storage, const builder_config_t& cfg, std::vector<uint8_t>&& src, std::vector<uint8_t>&& sym)  {return DocumentRaw(cfg,storage.buffer_i,storage.symbols_i);}
    static TreeRaw bind(const StorageFor& storage, const builder_config_t& cfg, std::vector<uint8_t>&& src, const void* label_offset=nullptr)  {return DocumentRaw(cfg,storage.buffer_i, {(uint8_t*)label_offset,std::span<uint8_t>::extent});}

};

template<>
struct StorageFor<Document>{
    std::vector<uint8_t> buffer_i;
    std::vector<uint8_t> symbols_i;

    StorageFor(const builder_config_t& cfg, std::vector<uint8_t>&& buf, std::vector<uint8_t>&& sym):buffer_i(buf),symbols_i(sym){}
    StorageFor(const builder_config_t& cfg, std::vector<uint8_t>&& buf, const void* label_offset=nullptr):buffer_i(buf){}

    static Tree bind(const StorageFor& storage, const builder_config_t& cfg, std::vector<uint8_t>&& src, std::vector<uint8_t>&& sym)  {return Document(DocumentRaw(cfg,storage.buffer_i,storage.symbols_i));}
    static Tree bind(const StorageFor& storage, const builder_config_t& cfg, std::vector<uint8_t>&& src, const void* label_offset=nullptr)  {return Document(DocumentRaw(cfg,storage.buffer_i, {(uint8_t*)label_offset,std::span<uint8_t>::extent}));}

};

namespace stored{
    using DocumentRaw = Stored<DocumentRaw>;
    using Document = Stored<Document>;
}

}