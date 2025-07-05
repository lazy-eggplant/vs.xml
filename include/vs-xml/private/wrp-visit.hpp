#pragma once
/**
 * @file wrp-visit.hpp
 * @author karurochari
 * @brief Visitor patterns for trees and derived
 * @date 2025-07-05
 *
 * @copyright Copyright (c) 2025
 *
 */

#include <vs-xml/commons.hpp>
#include <vs-xml/wrp-node.hpp>

namespace VS_XML_NS {
namespace wrp{

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
void visit(wrp::base_t<unknown_t> node, T1&& test, T2&& before, T2&& after, auto&&... args){
    while(true){
        if(node.ptr==nullptr)break;
        
        bool children_visited = test?!test(node, std::forward<decltype(args)>(args)...):true;
        if(before!=nullptr)before(node,std::forward<decltype(args)>(args)...);

        for(;;){
            if(node.ptr->has_children() && !children_visited){
                auto [l,r] =*node.children_range();
                node=l;
                children_visited = false;
                break;
            }
            if(node.ptr->has_next()){
                if(after!=nullptr)after(node,std::forward<decltype(args)>(args)...);
                node=node.next();
                children_visited = false;
                break;
            }
            if(node.ptr->has_parent()){
                if(after!=nullptr)after(node,std::forward<decltype(args)>(args)...);
                node.ptr = (const unknown_t*) node.parent().ptr;
                children_visited = true;
            }
            else{
                if(after!=nullptr)after(node,std::forward<decltype(args)>(args)...);
                node.ptr = nullptr;
                break;
            }
        }
    }
}

}
} // namespace VS_XML_NS