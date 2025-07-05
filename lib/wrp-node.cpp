#include <vs-xml/wrp-node.hpp>

namespace VS_XML_NS{
namespace wrp{

visitor_iterator& visitor_iterator::operator++(){
    auto& node = m_ptr.ptr;
    xml_assert(node!=nullptr, "Reached end of tree");
    bool children_visited = false;
    for(;;){
        if(node->has_children() && !children_visited){
            auto [l,r] =*node->children_range();
            node=l;
            children_visited = false;
            return *this;
        }
        if(node->has_next()){
            node=node->next();
            children_visited = false;
            return *this;
        }
        if(node->has_parent()){
            node = (const unknown_t*)node->parent();
            children_visited = true;
        }
        else{
            node = nullptr;
            return *this;
        }
    }
    return *this;
}  

}
}
