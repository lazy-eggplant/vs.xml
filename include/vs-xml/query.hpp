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

#include <vector>
#include <vs-xml/commons.hpp>

namespace VS_XML_NS{

struct query{
    struct token_t{
        enum type_t{
            VISIT_CHILDREN, VISIT_ATTRS, 
            MATCH_TYPE, 
            MATCH_NS, 
            MATCH_NAME, 
            MATCH_VALUE
        } type;

        //expression
    };

    struct exp{};

    std::vector<token_t> tokens;

    query& operator / (std::string_view){return *this;} //VISIT_CHILDREN & MATCH_NS/NAME
    query& operator > (std::string_view){return *this;} //VISIT_ATTRS & MATCH_NS/NAME

    //query& operator / (std::string_view){return *this;} //VISIT_CHILDREN & MATCH_NAME
    //query& operator > (std::string_view){return *this;} //VISIT_ATTRS & MATCH_NAME

    // document.root().children() | ns(...) | name(...)

    /*
        document.root() / "hello" / "*" / "ns:*" > "banana"
        document.root() / "hello" / "*" / "ns:*" > "banana"
    */

};


}