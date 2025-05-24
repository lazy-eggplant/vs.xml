#include "vs-xml/commons.hpp"
#include <string_view>
#include <variant>
#include <vs-xml/query.hpp>
#include <vs-xml/fwd/print.hpp>

namespace VS_XML_NS{
namespace query{

static inline bool expr_helper(const auto& pattern, const auto& check){
    if(std::holds_alternative<std::string_view>(pattern)){
        if(check.has_value() && *check==std::get<std::string_view>(pattern))return true; 
    }
    else if(std::holds_alternative<std::function<bool(std::string_view)>>(pattern)){
        if(!check.has_value() && std::get<std::function<bool(std::string_view)>>(pattern)(*check))return true;
    }
    else return true;
    return false;
}

std::generator<wrp::base_t<unknown_t>> traverse(wrp::base_t<unknown_t> root, std::vector<token_t>::const_iterator begin, std::vector<token_t>::const_iterator end) {
    for(auto current = begin;current!=end;current++){
        //Accept the current element
        if (std::holds_alternative<token_t::empty_t<token_t::ACCEPT>>(current->args)) {
            co_yield root;
            co_return;
        }
        //Continue on children if current is element
        else if (std::holds_alternative<token_t::empty_t<token_t::NEXT_LAYER>>(current->args)) {
            if(root.type()==type_t::ELEMENT) for (auto& child : root.children()) {
                for (auto n : traverse(child, current+1, end)) {
                    co_yield n;
                }
            }
            co_return;
        }
        //Continue from here on, FORK will just be consumed on the current branch AND on children if current is element, 
        else if (std::holds_alternative<token_t::empty_t<token_t::FORK>>(current->args)) {
            if(root.type()==type_t::ELEMENT) for (auto& child : root.children()) {
                for (auto n : traverse(child, current, end)) {
                    co_yield n;
                }
            }
            else co_return;
        }
        //Filter based on type
        else if (std::holds_alternative<token_t::type_filter_t<token_t::TYPE>>(current->args)) {
            auto type = std::get<token_t::type_filter_t<token_t::TYPE>>(current->args);
            bool match = false;
            switch(root.type()){
                case type_t::ELEMENT:
                    if(type.is_element)match=true;break;
                case type_t::TEXT:
                    if(type.is_text)match=true;break;
                case type_t::CDATA:
                    if(type.is_cdata)match=true;break;
                case type_t::COMMENT:
                    if(type.is_comment)match=true;break;
                case type_t::PROC:
                    if(type.is_proc)match=true;break;
                case type_t::MARKER:
                    if(type.is_marker)match=true;break;
                default:
                    break;
            }
            if(!match) co_return;   //All matches failing. Fail branch.
        }
        //Match NS
        else if ( std::holds_alternative<token_t::single_t<token_t::MATCH_NS>>(current->args) ){
            if(!expr_helper(std::get<token_t::single_t<token_t::MATCH_NS>>(current->args),root.ns())) co_return; 
        }
        //Match name
        else if ( std::holds_alternative<token_t::single_t<token_t::MATCH_NAME>>(current->args) ){
            if(!expr_helper(std::get<token_t::single_t<token_t::MATCH_NAME>>(current->args),root.name())) co_return; 
        }
        //Match value
        else if ( std::holds_alternative<token_t::single_t<token_t::MATCH_VALUE>>(current->args) ){
            if(!expr_helper(std::get<token_t::single_t<token_t::MATCH_VALUE>>(current->args),root.value())) co_return; 
        }
        //Match text, not implemented as .text() is missing upstream.
        /*
        else if (
            std::holds_alternative<token_t::single_t<token_t::MATCH_NAME>>(current->args) && 
            !expr_helper(std::get<token_t::single_t<token_t::MATCH_NAME>>(current->args),root.text())
        ) co_return; 
         */

        //TODO: right now attribute matching has k*O(n) complexity if k attributes must be tested.
        //By looking ahead it is possible to check if there are more attributes to be tested, and perform the operation in just O(n)
        //Match attribute
        else if (std::holds_alternative<token_t::attr_t<token_t::MATCH_ATTR>>(current->args)) {
            auto pattern = std::get<token_t::attr_t<token_t::MATCH_ATTR>>(current->args);
            bool found = false;
            for(auto& attr: root.attrs()){
                if(expr_helper(pattern.ns,attr.ns()) && expr_helper(pattern.name,attr.name()) && expr_helper(pattern.value,attr.value())){found=true;break;}
            }
            if(!found)co_return;
        }
        else{
            //Failed commands will prevent propagation.
            co_return;
        }
    }
}

}
}