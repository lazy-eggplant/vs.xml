/**
 * @file query-builder.cpp
 * @author karurochari
 * @brief Utility classes to build tree-queries & their linear representation.
 * @date 2025-05-28
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <expected>


#include <vs-xml/commons.hpp>
#include <vs-xml/private/wrp-impl.hpp>

namespace VS_XML_NS{
namespace query{

struct Token{
    typedef std::variant<std::monostate,std::string_view,std::function<bool(std::string_view)>>  operand_t;

    enum type_t : uint8_t{
        /*Empty*/
        ACCEPT, 
        NEXT, 
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

    using args_t = 
        std::variant<
            empty_t<ACCEPT>,
            empty_t<NEXT>,
            empty_t<FORK>,
            type_filter_t<TYPE>,
            single_t<MATCH_NS>,
            single_t<MATCH_NAME>,
            single_t<MATCH_VALUE>,
            single_t<MATCH_ALL_TEXT>,
            attr_t<MATCH_ATTR>
        > ;

    args_t args;

    constexpr Token(const args_t& t={}):args(t){}
};
    

struct QueryBuilder;
struct Query;
struct QueryResult;

struct QueryBuilder{
    struct Error;
    enum Type {IS, HAS};

    error_t begin_frame(Type type);
    error_t end_frame();

    error_t begin(Token token, size_t capture_ref = 0);
    error_t end();

    error_t inject(const Query query);

    std::expected<Query,error_t> close();

    private:
        size_t next_capture = 0;
};

}
}