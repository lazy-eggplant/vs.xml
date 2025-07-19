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

#include <vs-xml/commons.hpp>
#include <vs-xml/node.hpp>

#include <unordered_map>
#include <vector>
#include <span>
#include <optional>

struct RandomAccessIndex{
    using index_t = size_t;
    std::unordered_map<index_t, std::span<const VS_XML_NS::delta_ptr_t>>        nodes;
    std::vector<VS_XML_NS::delta_ptr_t>                                         buffer;

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
};

struct RandomAccessIndexLazy{
    private: 
    using index_t = size_t;
    std::unordered_map<index_t, std::span<const VS_XML_NS::delta_ptr_t>>        nodes;
    std::vector<VS_XML_NS::delta_ptr_t>                                         buffer;
    VS_XML_NS::Tree*                                                            base;

    decltype(nodes)::iterator compute(index_t idx){
        VS_XML_NS::unknown_t* node = (VS_XML_NS::unknown_t*)((uint8_t*)base + idx);
        for(auto child: node->children()){
            //TODO: add to buffer
        }
        //TODO: insert entry in the map
        //TODO: return iterator to the inserted entry
    }

    VS_XML_NS::unknown_t* resolve(index_t idx) const {return (VS_XML_NS::unknown_t*)((uint8_t*)base + idx);}
    
    public:

    std::optional<index_t> at(index_t idx, size_t count){
        if(resolve(idx)->type()!=xml::type_t::ELEMENT)return std::nullopt;

        auto it = nodes.find(idx);
        if(it==nodes.end())it = compute(idx);
        if(it->second.size()>count)return it->second[count];
        else return std::nullopt;
    }

    size_t count(index_t idx){
        if(resolve(idx)->type()!=xml::type_t::ELEMENT)return 0;

        auto it = nodes.find(idx);
        if(it==nodes.end()) it = compute(idx);
        return it->second.size();
    }

    std::span<const VS_XML_NS::delta_ptr_t> children(index_t idx){
        if(resolve(idx)->type()!=xml::type_t::ELEMENT)return {};

        auto it = nodes.find(idx);
        if(it==nodes.end()) it = compute(idx);
        return it->second;
    }

    //TODO: reserve method to preallocate memory
    void reserve(){

    }
};