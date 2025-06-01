#pragma once

/**
 * @file tree.hpp
 * @author karurochari
 * @brief Internal tree structure
 * @date 2025-05-01
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <cassert>
#include <cstdint>

#include <functional>
#include <expected>

#include <vector>
#include <string_view>
#include <span>

#include <ostream>

#include <vs-xml/commons.hpp>

namespace VS_XML_NS{

/**
 * @brief Base class for a tree. 
 * @warning Unless you need MAXIMUM PERFORMANCE, your are better using its derived VS_XML_NS::Tree
 * @details This one does not wrap returned XML entities nor string views, so you are left on your own to handle them, but you might gain few points in complex pipelines.
 * 
 */
struct TreeRaw{
    protected:
        std::vector<uint8_t> buffer_i;
        std::vector<uint8_t> symbols_i;

        std::span<uint8_t> buffer;      //Overlay of buffer_i, or external
        std::span<uint8_t> symbols;     //Overlay of symbols_i, or external

        builder_config_t configs;

    public:

    struct from_binary_error_t {
        enum Code {
            OK = 0,
            HeaderTooSmall,
            MagicMismatch,
            MajorVersionMismatch,
            MinorVersionTooHigh,
            TruncatedSpan,
            TreeOutOfBounds,
            SymbolsOutOfBounds,
            TooManyDocs,
            TypeMismatch
        } code;
        
        std::string_view msg();
    };
    

    std::function<bool(const unknown_t&, const unknown_t&)> def_order_node() const;
    std::function<bool(const attr_t&, const attr_t&)> def_order_attrs() const;

    /**
        * @brief Reorder (in-place) children of a node based on a custom ordering criterion.
        * 
        * @param ref Node whose children are to be ordered
        * @param fn Criterion to determine the order of nodes.
        * @return true if the operation is successful
        */
    bool reorder_children(
        const element_t* ref, 
        const std::function<bool(const unknown_t&, const unknown_t&)>& fn
    ){return false;}

    /**
        * @brief Reorder (in-place) attributes of a node based on a custom ordering criterion.
        * 
        * @param ref Node whose attributes are to be ordered
        * @param fn Criterion to determine the order of nodes.
        * @return true if the operation is successful
        */
    bool reorder(
        const std::function<bool(const attr_t&, const attr_t&)>& fn,
        const element_t* ref = nullptr, 
        bool recursive = true
    );

    inline bool reorder(const element_t* ref=nullptr, bool recursive = true){
        return reorder(def_order_attrs(),ref,recursive);
    }

    /**
        * @brief 
        * 
        * @param ref The node/leaf where to expand.
        * @param start 
        * @param end 
        * @return true 
        * @return false 
        */
    bool inject(const unknown_t* ref, const unknown_t* start, const unknown_t* end);

    bool inject_before(const unknown_t* ref, const unknown_t* start, const unknown_t* end);
    bool inject_after(const unknown_t* ref, const unknown_t* start, const unknown_t* end);

    /**
     * @brief Obtain a weak slice of a specific subtree.
     * 
     * @param ref the node where to start slicing.
     * @return const Tree 
     */
    const TreeRaw slice(const element_t* ref=nullptr) const;

    /**
     * @brief Return a perfect deep copy of the current tree structure.
     * @param ref the node where to start cloning.
     * @param reduce if true, a new 
     * @return Tree 
     */
    TreeRaw clone(const element_t* ref=nullptr, bool reduce=true) const;

    struct print_cfg_t{
        bool use_tabs = true;      //Else spaces
        bool use_quote = true;     //Else `
        bool use_lf = true;        //Else CRLF
        int spaces_in_tab = 4;

        print_cfg_t() {};

    };

    inline const unknown_t& root() const {return *(const unknown_t*)buffer.data();}

    inline bool print(std::ostream& out, const print_cfg_t& cfg = {})const{
        return print_h(out, cfg, (const unknown_t*)&root());
    }

    bool save_binary(std::ostream& out)const;

    [[nodiscard]] static std::expected<TreeRaw, TreeRaw::from_binary_error_t> from_binary(std::span<uint8_t> region);
    [[nodiscard]] static std::expected<const TreeRaw , TreeRaw::from_binary_error_t> from_binary(std::string_view region);

    inline std::string_view rsv(sv s) const{
        return std::string_view(s.base+(char*)symbols.data(),s.base+(char*)symbols.data()+s.length);
    }

    protected:

    /**
     * @brief Construct a new Tree object, with the list of strings being external. Make sure its lifetime contains the one of the Tree.
     */
    TreeRaw(const builder_config_t& cfg, std::vector<uint8_t>&& src, const void* label_offset=nullptr):
        buffer_i(src),symbols_i({}),
        buffer(buffer_i),symbols((uint8_t*)label_offset, std::span<uint8_t>::extent),
        configs(cfg){}

    /**
     * @brief Construct a new Tree object, owning its own table of strings
     */
    TreeRaw(const builder_config_t& cfg, std::vector<uint8_t>&& src, std::vector<uint8_t>&& sym):
        buffer_i(src),symbols_i(sym),
        buffer(buffer_i),symbols(symbols_i),
        configs(cfg){}
 
    /*
    //Weak, used when loading from disk
    Tree(std::span<uint8_t> src, void* label_offset):
        buffer(src),symbols((uint8_t*)label_offset, std::span<uint8_t>::extent){}
    */

    //Weak, used when loading from disk or creatung slices
    TreeRaw(const builder_config_t& cfg, std::span<uint8_t> src, std::span<uint8_t> sym):
        buffer(src),symbols(sym),configs(cfg){}

    TreeRaw(const builder_config_t& cfg, std::span<const uint8_t> src, std::span<const uint8_t> sym):
        buffer((uint8_t*)src.data(),src.size_bytes()),symbols((uint8_t*)sym.data(),sym.size_bytes()),configs(cfg){
    }

    bool print_h(std::ostream& out, const print_cfg_t& cfg = {}, const unknown_t* ptr=nullptr) const;

    bool reorder_h(
        const std::function<bool(const attr_t&, const attr_t&)>& fn,
        const element_t* ref, 
        bool recursive = true
    );

    template<builder_config_t>
    friend struct TreeBuilder;
    friend struct details::BuilderBase;
    friend wrp::sv;

    template <typename T>
    friend struct wrp::base_t;

    friend struct ArchiveRaw;
};


/**
 * @brief the tree class you should use.
 * @warning make sure you also check VS_XML_NS::Document as you are more likely going to use that one
 */
struct Tree : TreeRaw{
    private:
    using TreeRaw::rsv;
    using TreeRaw::clone;
    using TreeRaw::root;

    public:
    inline Tree(TreeRaw&& ref):TreeRaw(std::move(ref)){}
    inline Tree(const TreeRaw&& ref):TreeRaw(std::move(ref)){}

    inline const Tree slice(const element_t* ref=nullptr) const{return TreeRaw::slice(ref);}
    inline Tree clone(const element_t* ref=nullptr, bool reduce=true) const{return TreeRaw::clone(ref,reduce);}

    wrp::base_t<unknown_t> root() const;

    inline TreeRaw& downgrade(){return *this;}
};


}