#pragma once

/**
 * @file tree-iterator.hpp
 * @author karurochari
 * @brief Stackless iterator able to navigate a tree structure.
 * @date 2025-06-21
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <iterator>
#include <vs-xml/commons.hpp>
#include <vs-xml/tree.hpp>
#include <vs-xml/private/impl.hpp>

namespace VS_XML_NS{

struct TreeRawIterator{
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = const unknown_t;
    using pointer           = const value_type*;
    using reference         = const value_type&;

    inline TreeRawIterator(pointer ptr) : node(ptr) {}
    inline TreeRawIterator(reference r) : node(&r) {}
    inline TreeRawIterator() = default;
    inline TreeRawIterator(const TreeRawIterator&) = default;

    inline reference operator*() const { return *node; }
    inline pointer operator->() { return node; }

    inline TreeRawIterator& operator++() {
        xml_assert(node!=nullptr, "Reached end of tree");
        for(;;){
            if(node->has_children() && !children_visited){
                auto [l,r] =*node->children_range();
                node=l;
                children_visited = false;
                return *this;
            }
            if(node->has_next()){
                node=node->next();
                children_visited = false;
                return *this;
            }
            if(node->has_parent()){
                node = (const unknown_t*)node->parent();
                children_visited = true;
            }
            else{
                node = nullptr;
                return *this;
            }
        }
        return *this;
    }  

    /*
    //TODO: This one is probably not possible, as we don't know the aligned position of a node before the end of the range.
    |0......(|1.......|2.......)|3.......
    inline TreeIterator& operator--() {
        xml_assert(node!=nullptr, "Reached beginning of tree");
        for(;;){
            if(node->has_children() && !children_visited){
                auto [l,r] =*node->children_range();
                node=r; //-something
                children_visited = false;
                return *this;
            }
            if(node->has_prev()){
                node=node->prev();
                children_visited = false;
                return *this;
            }
            if(node->has_parent()){
                node = (const unknown_t*)node->parent();
                children_visited = true;
            }
            else{
                node = nullptr;
                return *this;
            }
        }
        return *this;
    }  
    inline TreeIterator operator--(int) { TreeIterator tmp = *this; --(*this); return tmp; }
    */

    inline TreeRawIterator operator++(int) { TreeRawIterator tmp = *this; ++(*this); return tmp; }

    inline friend bool operator== (const TreeRawIterator& a, const TreeRawIterator& b) { return a.node == b.node; };
    inline friend bool operator!= (const TreeRawIterator& a, const TreeRawIterator& b) { return a.node != b.node; };  


    TreeRawIterator(const TreeRaw& tree):tree(&tree){
        node = &tree.root();
    }

    private:
        const TreeRaw* tree;
        const unknown_t* node = nullptr;
        bool children_visited = false;
};

static_assert(std::forward_iterator<TreeRawIterator>);

};