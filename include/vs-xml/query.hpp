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
#include <optional>
#include <string_view>
#include <vector>
#include <vs-xml/commons.hpp>

namespace VS_XML_NS{

struct query{
    //This token data structure is quite heavy.
    //Sadly, there is no way to trivially optimize its layout as is, so it will suffice for now.
    struct token_t{
        struct operand_t{
            enum {IMM,FN,SKIP} mode;
            union{
                std::string_view sv;
                std::function<bool(std::string_view)> fn;
            };
        };

        enum type_t : uint8_t{
            /*Empty*/
            ACCEPT, 
            NEXT_LAYER, 
            TYPE_ELEMENT, TYPE_COMMENT, TYPE_PROC, TYPE_TEXT, TYPE_CDATA,
            /*Unary*/
            MATCH_NS, MATCH_NAME, MATCH_VALUE, MATCH_ALL_TEXT,
            /*Attr*/
            MATCH_ATTR
        } type;
        
        union{
            struct{} empty;
            struct{
                operand_t ns;
                operand_t name;
                operand_t value;
            } attr;
            operand_t unary;
        }args;
    };

    struct exp{};

    std::vector<token_t> tokens;

    query& operator / (std::string_view){return *this;}

};

//`is` generator references all nodes which are proven to complete the query
//`has` generator references all nodes for which sub-nodes are proven to complete their query.


}