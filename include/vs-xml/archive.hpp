#pragma once

/**
 * @file archive.hpp
 * @author karurochari
 * @brief A collection of multiple documents, stored in a single data structure. Archives will use shared symbols but split address space for each document.
 * @date 2025-05-21
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <cstdint>
#include <string_view>
#include <vs-xml/commons.hpp>
#include <vs-xml/document.hpp>

namespace VS_XML_NS{

/**
 * @brief Base class for an archive of documents.
 * @warning Unless you need MAXIMUM PERFORMANCE, your are better using its derived VS_XML_NS::Archive
 * @details This one does not wrap returned XML entities nor string views, so you are left on your own to handle them, but you might gain few points in complex pipelines.
 */
struct ArchiveRaw{
    private:
        std::span<binary_header_t::section_t> index;
        std::span<uint8_t> buffer;
        std::span<uint8_t> symbols;
        builder_config_t configs;

    public:

    using from_binary_error_t = TreeRaw::from_binary_error_t;

    ///Save a binary representation for this raw archive to an output stream.
    bool save_binary(std::ostream& out)const;

    ///Load this raw archive with data from a memory region, and return it unless failure.
    [[nodiscard]] static std::expected<ArchiveRaw, ArchiveRaw::from_binary_error_t> from_binary(std::span<uint8_t> region);

    ///Load this raw archive with data from a memory region, and return it unless failure.
    [[nodiscard]] static std::expected<const ArchiveRaw, ArchiveRaw::from_binary_error_t> from_binary(std::span<const uint8_t> region);

    ///Resolve a string view referred to the current symbol table to an absolute string view.
    [[nodiscard]] inline std::string_view rsv(sv s) const{
        return std::string_view(s.base+(char*)symbols.data(),s.base+(char*)symbols.data()+s.length);
    }

    ///Get the raw document in position idx if available
    [[nodiscard]] inline std::optional<DocumentRaw> get(size_t idx){
        //xml_assert(documents.size()>idx, "Out of bounds document selected");
        if(idx>index.size())return {};
        auto v = index[idx];
        return DocumentRaw(configs,std::span{buffer.data()+v.base,v.length},std::span{symbols.begin(),symbols.end()});
    }

    ///Get a constant raw document in position idx if available
    [[nodiscard]] inline std::optional<const DocumentRaw> get(size_t idx) const{
        //xml_assert(documents.size()>idx, "Out of bounds document selected");
        if(idx>index.size())return {};
        auto v = index[idx];
        return DocumentRaw(configs,std::span{buffer.data()+v.base,v.length},std::span{symbols.begin(),symbols.end()});
    }

    ///Get the raw document with a given name if it exists
    [[nodiscard]] inline std::optional<DocumentRaw> get(std::string_view name){
        //TODO introduce support for an ordered index?
        for(auto& doc: index){
            if(rsv({doc.name.base, doc.name.length})==name){
                return DocumentRaw(configs,std::span{buffer.data()+doc.base,doc.length},std::span{symbols.begin(),symbols.end()});
            }
        }
        return {};
    }

    ///Get the const raw document with a given name if it exists
    inline std::optional<const DocumentRaw> get(std::string_view name) const{
        for(auto& doc: index){
            if(rsv({doc.name.base, doc.name.length})==name){
                return DocumentRaw(configs,std::span{buffer.data()+doc.base,doc.length},symbols);
            }
        }
        return {};
    }

    inline ArchiveRaw(const builder_config_t& cfg, std::span<binary_header_t::section_t> docs, std::span<uint8_t> buff, std::span<uint8_t> syms = {(uint8_t*)nullptr, std::span<uint8_t>::extent}):
        index(docs),buffer(buff),symbols(syms),configs(cfg)
    {}

    inline ArchiveRaw(const builder_config_t& cfg, std::span<const binary_header_t::section_t> docs, std::span<const uint8_t> buff, std::span<const uint8_t> syms = {(const uint8_t*)nullptr, std::span<uint8_t>::extent}):
        index((binary_header_t::section_t*)docs.data(),docs.size()),
        buffer((uint8_t*)buff.data(),buff.size()),
        symbols((uint8_t*)syms.data(),syms.size()),
        configs(cfg)
    {}

    ///The number of items present in this archive
    [[nodiscard]] inline size_t items() const{return index.size();}

    private:
        ArchiveRaw(){};
};

/**
 * @brief the archive class you should use.
 */
struct Archive : ArchiveRaw{
    ///Get the document in position idx if available
    [[nodiscard]] inline std::optional<Document> get(size_t idx){
        auto tmp = ArchiveRaw::get(idx);
        if(tmp.has_value())return Document(std::move(*tmp));
        else return {};
    }

    ///Get a constant document in position idx if available
    [[nodiscard]] inline std::optional<const Document> get(size_t idx) const{
        auto tmp = ArchiveRaw::get(idx);
        if(tmp.has_value())return Document(std::move(*tmp));
        else return {};
    }

    ///Get the document with a given name if it exists
    [[nodiscard]] inline std::optional<Document> get(std::string_view name){
        auto tmp = ArchiveRaw::get(name);
        if(tmp.has_value())return Document(std::move(*tmp));
        else return {};
    }

    ///Get the const document with a given name if it exists
    [[nodiscard]] inline std::optional<const Document> get(std::string_view name) const{
        auto tmp = ArchiveRaw::get(name);
        if(tmp.has_value())return Document(std::move(*tmp));
        else return {};
    }

    ///Load this archive with data from a memory region, and return it unless failure.
    [[nodiscard]] static inline std::expected<Archive, ArchiveRaw::from_binary_error_t> from_binary(std::span<uint8_t> region){
        auto tmp = ArchiveRaw::from_binary(region);
        if(tmp.has_value())return Archive(std::move(*tmp));
        else return tmp;
    }
    
    ///Load this const archive with const data from a memory region, and return it unless failure.
    [[nodiscard]] static inline std::expected<const Archive, ArchiveRaw::from_binary_error_t> from_binary(std::span<const uint8_t> region){
        auto tmp = ArchiveRaw::from_binary(region);
        if(tmp.has_value())return Archive(std::move(*tmp));
        else return tmp;
    }

    inline Archive(ArchiveRaw&& ref):ArchiveRaw(std::move(ref)){}
    inline Archive(const ArchiveRaw&& ref):ArchiveRaw(std::move(ref)){}

    ///Reinterprete this archive as a raw archive.
    [[nodiscard]] inline ArchiveRaw& downgrade(){return *this;}

    ///Reinterprete this const archive as a const raw archive.
    [[nodiscard]] inline const ArchiveRaw& downgrade() const{return *this;}

    using ArchiveRaw::ArchiveRaw;
};

template<>
struct StorageFor<ArchiveRaw>{
    std::vector<binary_header_t::section_t> index_i;
    std::vector<uint8_t> buffer_i;
    std::vector<uint8_t> symbols_i;

    StorageFor(const builder_config_t& cfg, std::vector<binary_header_t::section_t>&& index, std::vector<uint8_t>&& buf, std::vector<uint8_t>&& sym):index_i(index),buffer_i(buf),symbols_i(sym){}
    StorageFor(const builder_config_t& cfg, std::vector<binary_header_t::section_t>&& index, std::vector<uint8_t>&& buf, const void* label_offset=nullptr):index_i(index),buffer_i(buf){}

    static ArchiveRaw bind(const StorageFor& storage, const builder_config_t& cfg, std::vector<binary_header_t::section_t>&& idx, std::vector<uint8_t>&& buff, std::vector<uint8_t>&& sym)  {return ArchiveRaw(cfg,storage.index_i,storage.buffer_i,storage.symbols_i);}
    static ArchiveRaw bind(const StorageFor& storage, const builder_config_t& cfg, std::vector<binary_header_t::section_t>&& idx, std::vector<uint8_t>&& buff, const void* label_offset=nullptr)  {return ArchiveRaw(cfg,storage.index_i,storage.buffer_i, {(uint8_t*)label_offset,std::span<uint8_t>::extent});}

};

template<>
struct StorageFor<Archive>{
    std::vector<binary_header_t::section_t> index_i;
    std::vector<uint8_t> buffer_i;
    std::vector<uint8_t> symbols_i;

    StorageFor(const builder_config_t& cfg, std::vector<binary_header_t::section_t>&& index, std::vector<uint8_t>&& buf, std::vector<uint8_t>&& sym):index_i(index),buffer_i(buf),symbols_i(sym){}
    StorageFor(const builder_config_t& cfg, std::vector<binary_header_t::section_t>&& index, std::vector<uint8_t>&& buf, const void* label_offset=nullptr):index_i(index),buffer_i(buf){}

    static ArchiveRaw bind(const StorageFor& storage, const builder_config_t& cfg, std::vector<binary_header_t::section_t>&& idx, std::vector<uint8_t>&& buff, std::vector<uint8_t>&& sym)  {return Archive(ArchiveRaw(cfg,storage.index_i,storage.buffer_i,storage.symbols_i));}
    static ArchiveRaw bind(const StorageFor& storage, const builder_config_t& cfg, std::vector<binary_header_t::section_t>&& idx, std::vector<uint8_t>&& buff, const void* label_offset=nullptr)  {return Archive(ArchiveRaw(cfg,storage.index_i,storage.buffer_i, {(uint8_t*)label_offset,std::span<uint8_t>::extent}));}

};

namespace stored{
    using ArchiveRaw = Stored<ArchiveRaw>;
    using Archive = Stored<Archive>;
}

}