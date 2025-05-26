#include "vs-xml/commons.hpp"
#include <string_view>
#include <variant>
#include <vs-xml/query.hpp>
#include <vs-xml/fwd/print.hpp>

namespace VS_XML_NS{
namespace query{


std::pair<std::string_view, std::string_view> split_on_colon(std::string_view input) {
    std::size_t pos = input.find(':');
    if (pos == std::string_view::npos) {
        // No colon found: return the whole string and an empty view.
        return {{}, input};
    } else {
        return {input.substr(0, pos), input.substr(pos + 1)};
    }
}
    
template<>
result_t is<0>(wrp::base_t<unknown_t> root, typename query_t<0>::container_type::const_iterator begin, typename query_t<0>::container_type::const_iterator end) ;

}
}