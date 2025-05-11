
#include <vs-xml/commons.hpp>
#include <vs-xml/impl.hpp>
#include <vs-xml/wrp-impl.hpp>

namespace VS_XML_NS{

wrp::base_t<element_t> WrpTree::root() const{return {*this, &Tree::root()};}

}