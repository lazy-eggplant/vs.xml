/**
 * @file rand-access.hpp
 * @author karurochari
 * @brief Utility class to enable an external index used to provide random access to children information
 * @date 2025-07-13
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include <iostream>
#include <vs-xml/commons.hpp>
#include <vs-xml/node.hpp>
#include <vs-xml/tree.hpp>

#include <vs-xml/fwd/unordered_map.hpp>
#include <vs-xml/fwd/vector.hpp>
#include <vs-xml/wrp-node.hpp>

#include <span>
#include <optional>
#include <iterator>  

struct RandomAccessIndex{
    using index_t = size_t;
    xml::unordered_map<index_t, std::span<const VS_XML_NS::delta_ptr_t>>        nodes;
    xml::vector<VS_XML_NS::delta_ptr_t>                                         buffer;

    std::optional<index_t> at(index_t idx, size_t count){
        auto it = nodes.find(idx);
        if(it==nodes.end())return std::nullopt;

        if(it->second.size()>count)return it->second[count];
        else return std::nullopt;
    }

    std::optional<size_t> count(index_t idx){
        auto it = nodes.find(idx);
        if(it==nodes.end()) return std::nullopt;
        return it->second.size();
    }

    std::optional<std::span<const VS_XML_NS::delta_ptr_t>> children(index_t idx){
        auto it = nodes.find(idx);
        if(it==nodes.end())return std::nullopt;
        return it->second;
    }

    struct reserve_t{
        size_t nodes = 0;
        size_t buffer = 0;
    };

    void reserve(const reserve_t& res){
        nodes.reserve(res.nodes);
        buffer.reserve(res.buffer);
    }
};

struct RandomAccessIndexLazy{
    private: 
    using address_t = size_t;
    using index_t = size_t;

    struct span{
        xml::xml_size_t start;
        xml::xml_count_t size;

        struct iterator {
            index_t pos;
            iterator(index_t p) : pos(p) {}
    
            // dereference returns the current index
            index_t operator*() const {
                return pos;
            }
    
            // pre-increment
            iterator& operator++() {
                ++pos;
                return *this;
            }
    
            // post-increment (optional)
            iterator operator++(int) {
                iterator tmp = *this;
                ++pos;
                return tmp;
            }

            //iterator operator[](int i) const{return pos+i;}
    
            // equality / inequality
            bool operator==(iterator const& o) const { return pos == o.pos; }
            bool operator!=(iterator const& o) const { return pos != o.pos; }
        };
    
        iterator begin() const {return start;}
        iterator end() const {return start+size;}
        
        iterator operator[](int i) const{
            assert(i>=start && i<start+size);
            return start+i;
        }
    };

    xml::unordered_map<address_t, span>      nodes;
    xml::vector<VS_XML_NS::delta_ptr_t>      buffer;
    const VS_XML_NS::TreeRaw*                base;

    decltype(nodes)::iterator compute(address_t idx){
        VS_XML_NS::unknown_t* node = resolve(idx);
        size_t initial_pos = buffer.size();

        for(auto& child: node->children()){
            buffer.push_back(child.rel_offset());
        }
        auto it = nodes.emplace(idx, span{initial_pos,buffer.size()-initial_pos});
        if(!it.second){/*THROW DO STUFF*/throw "ToBeDefined";}
        return it.first;
    }

    
    public:

    VS_XML_NS::unknown_t* resolve(address_t idx) const {return (VS_XML_NS::unknown_t*)((uint8_t*)&base->root() + idx);}
    VS_XML_NS::unknown_t* resolve(VS_XML_NS::unknown_t* root,index_t idx) const {
        return (VS_XML_NS::unknown_t*)((uint8_t*)root + buffer[idx]);
    }
    address_t resolve_rel(index_t idx) const {
        return buffer[idx];
    }

    RandomAccessIndexLazy(const VS_XML_NS::TreeRaw& tree){base=&tree;}

    size_t count(address_t idx){
        if(resolve(idx)->type()!=xml::type_t::ELEMENT)return 0;

        auto it = nodes.find(idx);
        if(it==nodes.end()) it = compute(idx);
        return it->second.size;
    }

    span children(address_t idx){
        if(resolve(idx)->type()!=xml::type_t::ELEMENT)return {};

        auto it = nodes.find(idx);
        if(it==nodes.end()) it = compute(idx);
        return it->second;
    }

    struct reserve_t{
        size_t nodes = 0;
        size_t buffer = 0;
    };

    void reserve(const reserve_t& res){
        nodes.reserve(res.nodes);
        buffer.reserve(res.buffer);
    }
};