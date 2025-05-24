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

//Temporary add custom implementation here later
#include <generator>
#include <functional>
#include <string_view>
#include <variant>
#include <vector>
#include <vs-xml/commons.hpp>
#include <vs-xml/private/wrp-impl.hpp>

namespace VS_XML_NS{

namespace query{

struct token_t{
    typedef std::variant<std::monostate,std::string_view,std::function<bool(std::string_view)>>  operand_t;

    enum type_t : uint8_t{
        /*Empty*/
        ACCEPT, 
        NEXT_LAYER, 
        FORK,
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
        uint8_t is_element:1 = false ;
        uint8_t is_comment:1 = false ;
        uint8_t is_proc:1 = false ;
        uint8_t is_text:1 = false ;
        uint8_t is_cdata:1 = false ;
        uint8_t is_marker:1 = false ;
    };

    template<type_t T>
    struct attr_t{
        operand_t name = std::monostate{};
        operand_t value = std::monostate{};
        operand_t ns = std::monostate{};
    };

    std::variant<
        empty_t<ACCEPT>,
        empty_t<NEXT_LAYER>,
        empty_t<FORK>,
        type_filter_t<TYPE>,
        single_t<MATCH_NS>,
        single_t<MATCH_NAME>,
        single_t<MATCH_VALUE>,
        single_t<MATCH_ALL_TEXT>,
        attr_t<MATCH_ATTR>
        > args;
};


constexpr static token_t accept() {
    return {token_t::empty_t<token_t::ACCEPT>{}};
}

constexpr static token_t fork() {
    return {token_t::empty_t<token_t::FORK>{}};
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

constexpr static token_t match_attr(token_t::attr_t<token_t::MATCH_ATTR> arg) {
    return {arg};
}


std::pair<std::string_view, std::string_view>
static split_on_colon(std::string_view input) {
    std::size_t pos = input.find(':');
    if (pos == std::string_view::npos) {
        // No colon found: return the whole string and an empty view.
        return {{}, input};
    } else {
        return {input.substr(0, pos), input.substr(pos + 1)};
    }
}

template<size_t N = 0>
struct query_t{
    //This token data structure is quite heavy.
    //Sadly, there is no way to trivially optimize its layout as is, so it will suffice for now.

    std::array<token_t,N> tokens;
    size_t current=0;


    constexpr query_t& operator * (std::string_view str){
        if(str=="*") return *this;
        else if(str=="**") return *this * fork();
        else{
            auto [ns,name] = split_on_colon(str);
            if(ns=="?" && name=="?") return *this * type({.is_element=true});
            else if (ns=="?") return *this * type({.is_element=true}) * match_name({name});
            else if (name=="?") return *this * type({.is_element=true}) * match_ns({ns});
            else return *this * type({.is_element=true}) * match_ns({ns}) * match_name({name});
        }
    }

    constexpr query_t& operator * (const token_t& tkn){tokens[current]=tkn;current++;return *this;}

    constexpr inline query_t& operator / (std::string_view str){
        return *this * next_layer() * str;
    }

    constexpr inline query_t& operator / (const token_t& tkn){
        return *this * next_layer() * tkn;
    }

};

template<>
struct query_t<0>{
    //This token data structure is quite heavy.
    //Sadly, there is no way to trivially optimize its layout as is, so it will suffice for now.

    std::vector<token_t> tokens;

    constexpr query_t& operator * (std::string_view str){
        if(str=="*") return *this;
        else if(str=="**") return *this * fork();
        else{
            auto [ns,name] = split_on_colon(str);
            if(ns=="?" && name=="?") return *this * type({.is_element=true});
            else if (ns=="?") return *this * type({.is_element=true}) * match_name({name});
            else if (name=="?") return *this * type({.is_element=true}) * match_ns({ns});
            else return *this * type({.is_element=true}) * match_ns({ns}) * match_name({name});
        }
    }

    constexpr query_t& operator * (const token_t& tkn){tokens.push_back(tkn);return *this;}

    constexpr inline query_t& operator / (std::string_view str){
        return *this * next_layer() * str;
    }

    constexpr inline query_t& operator / (const token_t& tkn){
        return *this * next_layer() * tkn;
    }

};

std::generator<wrp::base_t<unknown_t>> traverse(wrp::base_t<unknown_t> root, std::vector<token_t>::const_iterator begin, std::vector<token_t>::const_iterator end);

//template<size_t N>
inline std::generator<wrp::base_t<unknown_t>> traverse(wrp::base_t<element_t> root, const query_t<0>& query) {
    return traverse(*(wrp::base_t<unknown_t>*)&root, query.tokens.begin(), query.tokens.end());
}



}
//`is` generator references all nodes which are proven to complete the query
//`has` generator references all nodes for which sub-nodes are proven to complete their query.


}