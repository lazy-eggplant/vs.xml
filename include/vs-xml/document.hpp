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

#include <expected>

#include <vs-xml/commons.hpp>
#include <vs-xml/tree.hpp>
#include <vs-xml/tree-builder.hpp>
#include <vs-xml/node.hpp>

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

    bool print(std::ostream& out, const print_cfg_t& cfg = {})const;
    bool print_fast(std::ostream& out, const print_cfg_t& cfg = {})const;

    /**
     * @brief Return the root of the proper tree inside the document (if present)
     * 
     * @return std::optional<node_iterator> 
     */
    [[nodiscard]] std::optional<node_iterator> tree_root() const;

    [[nodiscard]] static std::expected<DocumentRaw,TreeRaw::from_binary_error_t> from_binary(std::span<uint8_t> region);
    [[nodiscard]] static const std::expected<const DocumentRaw,TreeRaw::from_binary_error_t> from_binary(std::span<const uint8_t> region);

    template<builder_config_t cfg>
    friend struct DocumentBuilder;

    //TODO: Replace with proper prototypes, and incapsulate the mv mechanism away as it is an implementation detail, not semantically correct.
    DocumentRaw(TreeRaw&& src);
    DocumentRaw(const TreeRaw&& src);

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
    Document(DocumentRaw&& ref);
    Document(const DocumentRaw&& ref);

    const Tree slice(const element_t* ref=nullptr) const;
    Tree clone(const element_t* ref=nullptr, bool reduce=true) const;

    wrp::base_t<unknown_t>  root();

    ///Cast this document as a raw document
    DocumentRaw& downgrade();

    ///Cast this const document as a const raw tree
    const DocumentRaw& downgrade() const;
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