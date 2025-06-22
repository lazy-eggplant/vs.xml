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


#include <string_view>
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
        MATCH_TYPE,
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
            type_filter_t<MATCH_TYPE>,
            single_t<MATCH_NS>,
            single_t<MATCH_NAME>,
            single_t<MATCH_VALUE>,
            single_t<MATCH_ALL_TEXT>,
            attr_t<MATCH_ATTR>
        > ;

    args_t args;
    std::string_view capture_name = {};

    constexpr Token(const args_t& t={}):args(t){}
};
    

struct QueryBuilder;
struct Query;
struct QueryResult;

struct QueryBuilder{
    struct Error;
    enum Type {IS, HAS};

    error_t begin_frame(Type type); //, void(*capturer)(std::string_view, xml_size_t, void*)=nullptr
    error_t end_frame();//Implicit accept

    error_t any(std::string_view capture = {});

    error_t begin(std::string_view capture = {});
    error_t end();

    error_t match_type(Token::type_filter_t<Token::type_t::MATCH_TYPE>);
    error_t match_ns(Token::single_t<Token::type_t::MATCH_NS>);
    error_t match_name(Token::single_t<Token::type_t::MATCH_NAME>);
    error_t match_value(Token::single_t<Token::type_t::MATCH_VALUE>);
    error_t match_all_text(Token::single_t<Token::type_t::MATCH_ALL_TEXT>);
    error_t match_attr(Token::attr_t<Token::type_t::MATCH_ATTR>, std::string_view capture = {});

    //Syntax sugar for ns/name/all_text in case of element
    //error_t element();

    //error_t fork();

    error_t inject(const Query query);

    [[nodiscard]] std::expected<Query,error_t> close();

    private:
        std::vector<Token>  tokens;
        std::vector<char>   symbols;
};

/*
std::map<std::string_view,xml_size_t,std::less<>> mapping;

QueryBuilder bld;
bld.begin_frame(IS, +[](std::string_view, xml_size_t, void*){....});
    bld.begin("capture-0");
        bld.match_type({.is_element=true});
        bld.match_name("tag-name");
        bld.match_attr({"ns","attr"}, "capture-0.attr");
    bld.close();
    bld.begin("capture-1");
        bld.match_type({.is_element=true});
        bld.match_name("tag-name");
        bld.match_attr({"ns","attr"}, "capture-0.attr");
    bld.close();
bld.end_frame();

*/

}
}