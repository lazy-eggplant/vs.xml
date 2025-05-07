
#include <cstddef>
#include <cassert>

#include <iterator>
#include <expected> 

#include <vs-xml/commons.hpp>
#include <vs-xml/impl.hpp>

#include <vs-xml/wrp-tree.hpp>

namespace xml{

//template struct wrp_base_t<node_t>;
//template struct wrp_base_t<comment_t>;
/*template Builder::error_t wrp_base_t<cdata_t>(std::string_view value);
template Builder::error_t wrp_base_t<text_t>(std::string_view value);
template Builder::error_t wrp_base_t<proc_t>(std::string_view value);
template Builder::error_t wrp_base_t<inject_t>(std::string_view value);
*/

wrp_base_t<node_t> WrpTree::root() const{return {*this, &Tree::root()};}

}