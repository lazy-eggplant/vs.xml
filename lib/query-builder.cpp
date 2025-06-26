#include <vs-xml/query-builder.hpp>

namespace VS_XML_NS{
namespace query{

QueryBuilder::error_t QueryBuilder::begin_frame(std::string_view name, Type type){return error_t::NOT_IMPLEMENTED;}
QueryBuilder::error_t QueryBuilder::end_frame(){return error_t::NOT_IMPLEMENTED;}

QueryBuilder::error_t QueryBuilder::any(std::string_view capture){return error_t::NOT_IMPLEMENTED;}

QueryBuilder::error_t QueryBuilder::begin(std::string_view capture){return error_t::NOT_IMPLEMENTED;}
QueryBuilder::error_t QueryBuilder::end(){return error_t::NOT_IMPLEMENTED;}

QueryBuilder::error_t QueryBuilder::match_type(Token::type_filter_t<Token::type_t::MATCH_TYPE> expr){return error_t::NOT_IMPLEMENTED;}
QueryBuilder::error_t QueryBuilder::match_ns(Token::single_t<Token::type_t::MATCH_NS> expr){return error_t::NOT_IMPLEMENTED;}
QueryBuilder::error_t QueryBuilder::match_name(Token::single_t<Token::type_t::MATCH_NAME> expr){return error_t::NOT_IMPLEMENTED;}
QueryBuilder::error_t QueryBuilder::match_value(Token::single_t<Token::type_t::MATCH_VALUE> expr){return error_t::NOT_IMPLEMENTED;}
QueryBuilder::error_t QueryBuilder::match_all_text(Token::single_t<Token::type_t::MATCH_ALL_TEXT> expr){return error_t::NOT_IMPLEMENTED;}
QueryBuilder::error_t QueryBuilder::match_attr(Token::attr_t<Token::type_t::MATCH_ATTR> expr, std::string_view capture){return error_t::NOT_IMPLEMENTED;}

}
}