#include "vs-xml/commons.hpp"
#include <vs-xml/node.hpp>
#include <vs-xml/fwd/format.hpp>

#define DISPATCH(X) \
if (type() == type_t::ELEMENT) return ((element_t*)this) -> X;\
else if (type() == type_t::TEXT) return ((text_t*)this)-> X;\
else if (type() == type_t::COMMENT) return ((comment_t*)this)-> X;\
else if (type() == type_t::PROC) return ((proc_t*)this)-> X;\
else if (type() == type_t::CDATA) return ((cdata_t*)this)-> X;\
else if (type() == type_t::MARKER) return ((marker_t*)this)-> X;\
else{\
    xml_assert(false,"Invalid XML thing type");\
    std::unreachable();\
}

#define CDISPATCH(X) \
if (type() == type_t::ELEMENT) return ((const element_t*)this) -> X;\
else if (type() == type_t::TEXT) return ((const text_t*)this)-> X;\
else if (type() == type_t::COMMENT) return ((const comment_t*)this)-> X;\
else if (type() == type_t::PROC) return ((const proc_t*)this)-> X;\
else if (type() == type_t::CDATA) return ((const cdata_t*)this)-> X;\
else if (type() == type_t::MARKER) return ((const marker_t*)this)-> X;\
else{\
    xml_assert(false,VS_XML_NS::format("Invalid XML thing type {}",(int)type()).c_str());\
    std::unreachable();\
}

namespace VS_XML_NS {
    void unknown_t::set_parent(element_t* parent){DISPATCH(set_parent(parent));}
    void unknown_t::set_prev(unknown_t* prev){DISPATCH(set_prev(prev));}
    void unknown_t::set_next(unknown_t* next){DISPATCH(set_next(next));}

    std::expected<sv,feature_t> unknown_t::ns() const {CDISPATCH(ns());}
    std::expected<sv,feature_t> unknown_t::name() const {CDISPATCH(name());}
    std::expected<sv,feature_t> unknown_t::value() const {CDISPATCH(value());}
    std::expected<void,feature_t> unknown_t::text_range() const {CDISPATCH(text_range());}

    std::expected<std::pair<const unknown_t*, const unknown_t*>,feature_t> unknown_t::children_range() const {CDISPATCH(children_range());}
    std::expected<std::pair<const attr_t*, const attr_t*>,feature_t> unknown_t::attrs_range() const {CDISPATCH(attrs_range());}

    const delta_ptr_t unknown_t::rel_offset() const {CDISPATCH(rel_offset());}
    const element_t* unknown_t::parent() const {CDISPATCH(parent());}
    const unknown_t* unknown_t::prev() const {CDISPATCH(prev());}
    const unknown_t* unknown_t::next() const {CDISPATCH(next());}

    bool unknown_t::has_children() const {CDISPATCH(has_children());}
    bool unknown_t::has_parent() const {CDISPATCH(has_parent());}
    bool unknown_t::has_prev() const {CDISPATCH(has_prev());}
    bool unknown_t::has_next() const {CDISPATCH(has_next());}

    visitor_iterator& visitor_iterator::operator++(){
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

#undef DISPATCH
#undef CDISPATCH