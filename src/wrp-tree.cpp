
#include <cassert>

#include <expected> 

#include <vs-xml/commons.hpp>
#include <vs-xml/impl.hpp>

#include <vs-xml/wrp-tree.hpp>

namespace VS_XML_NS{

details::wrp_base_t<node_t> WrpTree::root() const{return {*this, &Tree::root()};}

}