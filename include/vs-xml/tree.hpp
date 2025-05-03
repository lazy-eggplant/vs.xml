#pragma once

#include <cassert>
#include <cstdint>

#include <functional>
#include <expected>

#include <vector>
#include <string_view>

#include <ostream>

#include "commons.hpp"

namespace xml{

struct Tree{
    std::vector<uint8_t> buffer;
    std::vector<uint8_t> symbols;
    const node_t& root;
    void* label_offset;

    Tree(std::vector<uint8_t>&& src):buffer(src),symbols({}),root(*(node_t*)src.data()),label_offset(nullptr){}

    Tree(std::vector<uint8_t>&& src, std::vector<uint8_t>&& sym):buffer(src),symbols(sym),root(*(node_t*)src.data()),label_offset(symbols.data()){}

    /**
        * @brief Reorder (in-place) children of a node based on a custom ordering criterion.
        * 
        * @param ref Node whose children are to be ordered
        * @param fn Criterion to determine the order of nodes. -1 for left<right, 0 for equals, 1 for left>right
        * @return true if the operation is successful
        */
    bool reorder_children(const node_t* ref, const std::function<int(const unknown_t*, const unknown_t*)>& fn){return false;}

    /**
        * @brief Reorder (in-place) attributes of a node based on a custom ordering criterion.
        * 
        * @param ref Node whose attributes are to be ordered
        * @param fn Criterion to determine the order of nodes. -1 for left<right, 0 for equals, 1 for left>right
        * @return true if the operation is successful
        */
    bool reorder(const node_t* ref, const std::function<int(const attr_t*, const attr_t*)>& fn){return false;}

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
        * @brief Return a perfect deep copy of the current tree structure.
        * 
        * @return tree 
        */
    Tree clone() const;

    struct print_cfg_t{};

    inline bool print(std::ostream& out, const print_cfg_t& cfg = {})const{
        return print_h(out, cfg, (const unknown_t*)&root);
    }

    inline std::string_view rsv(sv s) const{
        return std::string_view(s.base+(char*)label_offset,s.base+(char*)label_offset+s.length);
    }

    private:

    bool print_h(std::ostream& out, const print_cfg_t& cfg = {}, const unknown_t* ptr=nullptr) const;
};


}