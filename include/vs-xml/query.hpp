#pragma once

/**
 * @file query.hpp
 * @author karurochari
 * @brief Queries on a tree.
 * @date 2025-05-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <functional>
#include <string_view>
#include <variant>
#include <vector>
#include <vs-xml/commons.hpp>
#include <vs-xml/private/wrp-impl.hpp>

//Temporary add custom implementation here later
#include <generator>

namespace VS_XML_NS{

namespace query{

struct token_t{
    typedef std::variant<std::monostate,std::string_view,std::function<bool(std::string_view)>>  operand_t;

    enum type_t : uint8_t{
        /*Empty*/
        ACCEPT, 
        NEXT_LAYER, 
        /*type_filter*/
        TYPE,
        /*Unary sv*/
        MATCH_NS, MATCH_NAME, MATCH_VALUE, MATCH_ALL_TEXT,
        /*Attr*/
        MATCH_ATTR
    };
    
    template<type_t T>
    struct empty_t{};

    template<type_t T>
    struct single_t : operand_t{};

    template<type_t T>
    struct type_filter_t{
        uint8_t is_element:1 = true ;
        uint8_t is_commnent:1 = true ;
        uint8_t is_proc:1 = true ;
        uint8_t is_text:1 = true ;
        uint8_t is_cdata:1 = true ;
    };

    template<type_t T>
    struct attr_t{
        operand_t ns = std::monostate{};
        operand_t name = std::monostate{};
        operand_t value = std::monostate{};
    };

    std::variant<
        empty_t<ACCEPT>,
        empty_t<NEXT_LAYER>,
        type_filter_t<TYPE>,
        single_t<MATCH_NS>,
        single_t<MATCH_NAME>,
        single_t<MATCH_VALUE>,
        single_t<MATCH_ALL_TEXT>,
        attr_t<MATCH_ATTR>
        > args;
};

template<size_t N = 0>
struct query_t{
    //This token data structure is quite heavy.
    //Sadly, there is no way to trivially optimize its layout as is, so it will suffice for now.

    std::array<token_t,N> tokens;
    size_t current=0;

    constexpr query_t& operator / (std::string_view){return *this;}
    constexpr query_t& operator / (const token_t& tkn){tokens[current]=tkn;current++;return *this;}

};

template<>
struct query_t<0>{
    //This token data structure is quite heavy.
    //Sadly, there is no way to trivially optimize its layout as is, so it will suffice for now.

    std::vector<token_t> tokens;


    query_t& operator / (std::string_view){return *this;}
    query_t& operator / (const token_t& tkn){tokens.push_back(tkn);return *this;}

};

constexpr static token_t accept() {
    return {token_t::empty_t<token_t::ACCEPT>{}};
}

constexpr static token_t next_layer() {
    return {token_t::empty_t<token_t::NEXT_LAYER>{}};
}

constexpr static token_t type(token_t::type_filter_t<token_t::TYPE> arg) {
    return {arg};
}

constexpr static token_t match_ns(token_t::single_t<token_t::MATCH_NS> arg) {
    return {arg};
}

constexpr static token_t match_name(token_t::single_t<token_t::MATCH_NAME> arg) {
    return {arg};
}

constexpr static token_t match_value(token_t::single_t<token_t::MATCH_VALUE> arg) {
    return {arg};
}

constexpr static token_t match_all_text(token_t::single_t<token_t::MATCH_ALL_TEXT> arg) {
    return {arg};
}

constexpr static token_t match_all_text(token_t::attr_t<token_t::MATCH_ATTR> arg) {
    return {arg};
}

std::generator<wrp::base_t<unknown_t>> traverse(wrp::base_t<unknown_t> root, std::vector<token_t>::iterator begin, std::vector<token_t>::iterator end);

//template<size_t N>
inline std::generator<wrp::base_t<unknown_t>> traverse(wrp::base_t<unknown_t> root, query_t<0> query) {
    return traverse(root, query.tokens.begin(), query.tokens.end());
}



}
//`is` generator references all nodes which are proven to complete the query
//`has` generator references all nodes for which sub-nodes are proven to complete their query.


}