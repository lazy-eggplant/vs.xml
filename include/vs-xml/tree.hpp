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

#include "commons.hpp"

namespace VS_XML_NS{

struct Tree{
    struct __attribute__ ((packed)) serialized_header_t{
        const char magic[4] = {'$','X','M','L'};
        uint32_t reserved_cfg = 0;
        size_t offset_symbols;
        size_t offset_end;
    };

    protected:
        std::vector<uint8_t> buffer_i;
        std::vector<uint8_t> symbols_i;

        std::span<uint8_t> buffer;      //Overlay of buffer_i, or external
        std::span<uint8_t> symbols;     //Overlay of symbols_i, or external

        const builder_config_t configs;

    public:

    std::function<bool(const unknown_t&, const unknown_t&)> def_order_node();
    std::function<bool(const attr_t&, const attr_t&)> def_order_attrs();

    /**
        * @brief Reorder (in-place) children of a node based on a custom ordering criterion.
        * 
        * @param ref Node whose children are to be ordered
        * @param fn Criterion to determine the order of nodes. -1 for left<right, 0 for equals, 1 for left>right
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
        * @param fn Criterion to determine the order of nodes. -1 for left<right, 0 for equals, 1 for left>right
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
    const Tree slice(const element_t* ref=nullptr) const;

    /**
     * @brief Return a perfect deep copy of the current tree structure.
    * @param ref the node where to start cloning.
    * @param reduce if true, a new 
    * @return Tree 
     */
    Tree clone(const element_t* ref=nullptr, bool reduce=true) const;

    struct print_cfg_t{};

    inline const element_t& root() const {return *(const element_t*)buffer.data();}

    inline bool print(std::ostream& out, const print_cfg_t& cfg = {})const{
        return print_h(out, cfg, (const unknown_t*)&root());
    }

    bool save_binary(std::ostream& out)const;
    Tree(const builder_config_t& cfg,std::span<uint8_t> region);

    inline std::string_view rsv(sv s) const{
        return std::string_view(s.base+(char*)symbols.data(),s.base+(char*)symbols.data()+s.length);
    }

    private:

    /**
     * @brief Construct a new Tree object, with the list of strings being external. Make sure its lifetime contains the one of the Tree.
     */
    Tree(const builder_config_t& cfg, std::vector<uint8_t>&& src, void* label_offset=nullptr):
        buffer_i(src),symbols_i({}),
        buffer(buffer_i),symbols((uint8_t*)label_offset, std::span<uint8_t>::extent),
        configs(cfg){}

    /**
     * @brief Construct a new Tree object, owning its own table of strings
     */
    Tree(const builder_config_t& cfg, std::vector<uint8_t>&& src, std::vector<uint8_t>&& sym):
        buffer_i(src),symbols_i(sym),
        buffer(buffer_i),symbols(symbols_i),
        configs(cfg){}
 
    /*
    //Weak, used when loading from disk
    Tree(std::span<uint8_t> src, void* label_offset):
        buffer(src),symbols((uint8_t*)label_offset, std::span<uint8_t>::extent){}
    */

    //Weak, used when loading from disk or creatung slices
    Tree(const builder_config_t& cfg, std::span<uint8_t> src, std::span<uint8_t> sym):
        buffer(src),symbols(sym),configs(cfg){}


    bool print_h(std::ostream& out, const print_cfg_t& cfg = {}, const unknown_t* ptr=nullptr) const;
    bool reorder_h(
        const std::function<bool(const attr_t&, const attr_t&)>& fn,
        const element_t* ref, 
        bool recursive = true
    );

    template<builder_config_t>
    friend struct Builder;
    friend struct details::BuilderBase;
};



struct WrpTree : Tree{
    private:
    using Tree::rsv;
    using Tree::clone;
    using Tree::root;

    public:
    inline WrpTree(Tree&& ref):Tree(std::move(ref)){}
    inline WrpTree(const Tree&& ref):Tree(std::move(ref)){}

    inline const WrpTree slice(const element_t* ref=nullptr) const{return Tree::slice(ref);}
    inline WrpTree clone(const element_t* ref=nullptr, bool reduce=true) const{return Tree::clone(ref,reduce);}

    wrp::base_t<element_t> root() const;

    inline Tree& downgrade(){return *this;}
};


}