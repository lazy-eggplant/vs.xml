#include <vs-xml/document.hpp>

namespace VS_XML_NS{

bool DocumentRaw::print(std::ostream& out, const print_cfg_t& cfg)const{
    for(auto& it: TreeRaw::root().children()){
        if(!TreeRaw::print(out, cfg, &it))return false;
    }
    return true;
}

bool DocumentRaw::print_fast(std::ostream& out, const print_cfg_t& cfg)const{
    for(auto& it: TreeRaw::root().children()){
        if(!TreeRaw::print_fast(out, cfg, &it))return false;
    }
    return true;
}

/**
    * @brief Return the root of the proper tree inside the document (if present)
    * 
    * @return std::optional<node_iterator> 
    */
[[nodiscard]] std::optional<node_iterator>  DocumentRaw::tree_root() const{
    auto c  = TreeRaw::root().children();
    auto it = std::ranges::find_if(c,[](auto e)static{return e.type()==type_t::ELEMENT;});
    if(it!=c.end()) return it;
    return {};
}

[[nodiscard]] std::expected<DocumentRaw,TreeRaw::from_binary_error_t>  DocumentRaw::from_binary(std::span<uint8_t> region){
    std::expected<TreeRaw, TreeRaw::from_binary_error_t> t = TreeRaw::from_binary(region); 
    if(!t.has_value())return std::unexpected(t.error()); 
    else return DocumentRaw(std::move(*t));
}
[[nodiscard]] const std::expected<const DocumentRaw,TreeRaw::from_binary_error_t>  DocumentRaw::from_binary(std::span<const uint8_t> region){
    std::expected<const TreeRaw, TreeRaw::from_binary_error_t> t = TreeRaw::from_binary(region); 
    if(!t.has_value())return std::unexpected(t.error()); 
    else return DocumentRaw(std::move(*t));
}


//TODO: Replace with proper prototypes, and incapsulate the mv mechanism away as it is an implementation detail, not semantically correct.
DocumentRaw::DocumentRaw(TreeRaw&& src):TreeRaw(src){}
DocumentRaw::DocumentRaw(const TreeRaw&& src):TreeRaw(src){}


Document::Document(DocumentRaw&& ref):DocumentRaw(std::move(ref)){}
Document::Document(const DocumentRaw&& ref):DocumentRaw(std::move(ref)){}

const Tree Document::slice(const element_t* ref) const{return DocumentRaw::slice(ref);}
Tree Document::clone(const element_t* ref, bool reduce) const{return DocumentRaw::clone(ref,reduce);}

wrp::base_t<unknown_t> Document::root() {return wrp::base_t<unknown_t>{*(const TreeRaw*)this, &TreeRaw::root()};}

///Cast this document as a raw document
DocumentRaw& Document::downgrade(){return *this;}

///Cast this const document as a const raw tree
const DocumentRaw& Document::downgrade() const{return *this;}

}