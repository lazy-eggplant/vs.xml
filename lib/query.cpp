#include <vs-xml/query.hpp>
#include <vs-xml/fwd/print.hpp>

namespace VS_XML_NS{
namespace query{

std::generator<wrp::base_t<unknown_t>> traverse(wrp::base_t<unknown_t> root, std::vector<token_t>::const_iterator begin, std::vector<token_t>::const_iterator end) {
    if(begin==end)co_return;
    if (std::holds_alternative<token_t::empty_t<token_t::ACCEPT>>(begin->args)) {
        xml::print("ACCEPT: ");
        co_yield root;
    }
    if(root.type()==type_t::ELEMENT) for (auto& child : root.children()) {
        for (auto n : traverse(child, begin, end)) {
            co_yield n;
        }
    }
}

}
}