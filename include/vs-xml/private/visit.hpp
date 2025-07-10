#pragma once
/**
 * @file visit.hpp
 * @author karurochari
 * @brief Visitor patterns for trees and derived
 * @date 2025-07-05
 *
 * @copyright Copyright (c) 2025
 *
 */

#include <vs-xml/commons.hpp>
#include <vs-xml/node.hpp>

namespace VS_XML_NS {

/**
 * @brief Visit all nodes starting from node. Evaluate if children should be
 * considered by evaluating fn
 *
 * @tparam T1 implicit type for test function
 * @tparam T2 implicit type for before/after
 * @param node the starting node
 * @param test the function checking if children should be explored for this
 * node (can have side-effects).
 * @param before the function with side-effects run when entering a node.
 * @param after the function with side-effects when exiting a node.
 * @param args additional args if wanted
 */
template<typename T1, typename T2>
void visit(const unknown_t *node, T1&& test={}, T2&& before = {}, T2&& after = {}, auto&&... args){
    if(node==nullptr)return;

    auto initial=
        node->has_next()?
            (const unknown_t *)node->next():
            (const unknown_t *)node->parent();
            
    while(true){
        if(node==initial)break;
        if(before!=nullptr)before(node, std::forward<decltype(args)>(args)...);

        bool children_visited = test?!test(node, std::forward<decltype(args)>(args)...):true;
        for(;;){
            if(node->has_children() && !children_visited){
                auto [l,r] =*node->children_range();
                node=l;
                children_visited = false;
                break;
            }
            if(node->has_next()){
                if(after!=nullptr)after(node,std::forward<decltype(args)>(args)...);
                node=node->next();
                children_visited = false;
                break;
            }
            if(node->has_parent()){
                if(after!=nullptr)after(node,std::forward<decltype(args)>(args)...);
                node = (const unknown_t*)node->parent();
                children_visited = true;
                if(node==initial)break;
            }
            else{
                if(after!=nullptr)after(node,std::forward<decltype(args)>(args)...);
                node = initial;
                break;
            }
        }   
    }
}

} // namespace VS_XML_NS