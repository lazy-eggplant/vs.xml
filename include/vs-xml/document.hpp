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
#include  <vs-xml/builder.hpp>

namespace VS_XML_NS{

//TODO: Implemented but very lass on constraints. No wrapping nor checks which are needed to store a valid XML.

template<builder_config_t cfg>
struct DocBuilder;

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
    friend struct DocBuilder;

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

    /**
     * @brief Downgrade the current document into its base VS_XML_NS::DocumentRaw.
     * 
     * @return DocumentRaw& 
     */
    inline DocumentRaw& downgrade(){return *this;}
};


template<>
struct StorageFor<DocumentRaw>{
    std::vector<uint8_t> buffer_i;
    std::vector<uint8_t> symbols_i;

    StorageFor(const builder_config_t& cfg, std::vector<uint8_t>&& buf, std::vector<uint8_t>&& sym):buffer_i(buf),symbols_i(sym){}
    StorageFor(const builder_config_t& cfg, std::vector<uint8_t>&& buf, const void* label_offset=nullptr):buffer_i(buf){}

    static TreeRaw bind(const StorageFor& storage, const builder_config_t& cfg, std::vector<uint8_t>&& src, std::vector<uint8_t>&& sym)  {return DocumentRaw(cfg,storage.buffer_i,storage.symbols_i);}
    static TreeRaw bind(const StorageFor& storage, const builder_config_t& cfg, std::vector<uint8_t>&& src, const void* label_offset=nullptr)  {return DocumentRaw(cfg,storage.buffer_i);}

};

template<>
struct StorageFor<Document>{
    std::vector<uint8_t> buffer_i;
    std::vector<uint8_t> symbols_i;

    StorageFor(const builder_config_t& cfg, std::vector<uint8_t>&& buf, std::vector<uint8_t>&& sym):buffer_i(buf),symbols_i(sym){}
    StorageFor(const builder_config_t& cfg, std::vector<uint8_t>&& buf, const void* label_offset=nullptr):buffer_i(buf){}

    static Tree bind(const StorageFor& storage, const builder_config_t& cfg, std::vector<uint8_t>&& src, std::vector<uint8_t>&& sym)  {return Document(DocumentRaw(cfg,storage.buffer_i,storage.symbols_i));}
    static Tree bind(const StorageFor& storage, const builder_config_t& cfg, std::vector<uint8_t>&& src, const void* label_offset=nullptr)  {return Document(DocumentRaw(cfg,storage.buffer_i));}

};

namespace stored{
    using DocumentRaw = Stored<DocumentRaw>;
    using Document = Stored<Document>;
}

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

    [[nodiscard]] std::expected<std::pair<sv,std::vector<uint8_t>>,details::BuilderBase::error_t> close_frame(std::string_view name=""){
        this->end();
        auto tmp = TreeBuilder<configs>::close_frame(name);
        this->begin("ROOT");
        return tmp;
    }
    
    [[nodiscard]] std::optional<std::vector<uint8_t>> extract_symbols(){
        this->end();
        details::BuilderBase::close();
        return TreeBuilder<configs>::extract_symbols();
    }
};



}