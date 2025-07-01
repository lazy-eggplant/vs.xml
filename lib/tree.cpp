#include <cstring>
#include <algorithm>
#include <string_view>

#include <vs-xml/commons.hpp>
#include <vs-xml/tree.hpp>
#include <vs-xml/private/impl.hpp>
#include <vs-xml/private/wrp-impl.hpp>
#include <vs-xml/serializer.hpp>

#include <vs-xml/fwd/print.hpp>

namespace VS_XML_NS{

std::function<bool(const unknown_t&, const unknown_t&)> TreeRaw::def_order_node() const{
    #if VS_XML_NO_EXCEPT != true
        throw std::runtime_error("Not implemented");
    #else
        //TODO: tidy logic
        exit(1);
    #endif
}

std::function<bool(const attr_t&, const attr_t&)> TreeRaw::def_order_attrs() const{
    return [this](const attr_t& a, const attr_t& b){
        {
            auto va = rsv(*a.ns()), vb= rsv(*b.ns());
            if(va<vb)return true;
            else if(va>vb)return false;
        }
        {
            auto va = rsv(*a.name()), vb= rsv(*b.name());
            if(va<vb)return true;
            else if(va>vb)return false;
        }
        return false;
    };
}

bool TreeRaw::reorder(const std::function<bool(const attr_t&, const attr_t&)>& fn, const element_t* ref,  bool recursive){
    if(ref==nullptr){
        xml_assert(root().type()==type_t::ELEMENT);
        ref=(const element_t*)&root();
    }

    xml_assert((uint8_t*)ref>=(uint8_t*)buffer.data() && (uint8_t*)ref<(uint8_t*)buffer.data()+buffer.size());
    xml_assert(ref->type()==type_t::ELEMENT);
    return reorder_h(def_order_attrs(),ref,recursive);
}

bool TreeRaw::reorder_h(const std::function<bool(const attr_t&, const attr_t&)>& fn, const element_t* ref,  bool recursive){
    //Speed could be improved by using an intermediate swapping function, but attr_t elements are small enough that it might not be worthed.
    //TODO: at some point, convert it not to be recursive.

    if(ref->type()!=type_t::ELEMENT)return false;

    std::sort((attr_t*)ref->_attrs,(attr_t*)ref->_attrs+ref->attrs_count,fn);

    if(recursive){
        for(auto &i: ref->children()){
            if(i.type()==type_t::ELEMENT)
            reorder_h(fn,&(const element_t&)i,recursive);
        }
    }

    return true;
};


bool TreeRaw::print_h(std::ostream& out, const print_cfg_t& cfg, const unknown_t* ptr) const{
    //TODO: at some point, convert it not to be recursive.
    if(ptr->type()==type_t::ELEMENT){
        if(ptr->children_range()->first==ptr->children_range()->second){
            VS_XML_NS::print(out,"<{}{}{}", rsv(*ptr->ns()), rsv(*ptr->ns())==""?"":":", rsv(*ptr->name()));
            for(auto& i : ptr->attrs()){
                if(!configs.raw_strings){
                    auto t = serialize::to_xml_attr_2(rsv(*i.value()));
                    if(!t.has_value()){/*TODO: Error*/}
                    auto tt = t.value_or(std::string_view(""));
                    std::string_view  sv = std::holds_alternative<std::string>(tt)?std::get<std::string>(tt):std::get<std::string_view>(tt);
                    VS_XML_NS::print(out," {}{}{}=\"{}\"", rsv(*i.ns()), rsv(*i.ns())==""?"":":", rsv(*i.name()), sv);
                }
                else{
                    VS_XML_NS::print(out," {}{}{}=\"{}\"", rsv(*i.ns()), rsv(*i.ns())==""?"":":", rsv(*i.name()), rsv(*i.value()));
                }
            }
            out << "/>";
        }
        else{
            VS_XML_NS::print(out,"<{}{}{}", rsv(*ptr->ns()), rsv(*ptr->ns())==""?"":":", rsv(*ptr->name()));
            for(auto& i : ptr->attrs()){
                if(!configs.raw_strings){
                    auto t = serialize::to_xml_attr_2(rsv(*i.value()));
                    if(!t.has_value()){/*TODO: Error*/}
                    auto tt = t.value_or(std::string_view(""));
                    std::string_view  sv = std::holds_alternative<std::string>(tt)?std::get<std::string>(tt):std::get<std::string_view>(tt);
                    VS_XML_NS::print(out," {}{}{}=\"{}\"", rsv(*i.ns()), rsv(*i.ns())==""?"":":", rsv(*i.name()), sv);
                }
                else{
                    VS_XML_NS::print(out," {}{}{}=\"{}\"", rsv(*i.ns()), rsv(*i.ns())==""?"":":", rsv(*i.name()), rsv(*i.value()));
                }
            }
            out << ">";
            for(auto& i : ptr->children()){
                print_h(out,cfg,&i);
            }
            VS_XML_NS::print(out,"</{}{}{}>", rsv(*ptr->ns()), rsv(*ptr->ns())==""?"":":", rsv(*ptr->name()));
        }
    }
    else if(ptr->type()==type_t::CDATA){
        if(!configs.raw_strings){
            auto t = serialize::to_xml_cdata(rsv(*ptr->value()));
            if(!t.has_value()){/*TODO: Error*/}
            auto tt = t.value_or(std::string_view(""));
            std::string_view sv = std::holds_alternative<std::string>(tt)?std::get<std::string>(tt):std::get<std::string_view>(tt);
            VS_XML_NS::print(out,"<![CDATA[{}]]>",sv);
        }
        else{
            VS_XML_NS::print(out,"<![CDATA[{}]]>",rsv(*ptr->value()));
        }
    }
    else if(ptr->type()==type_t::COMMENT){
        if(!configs.raw_strings){
            auto t = serialize::to_xml_comment(rsv(*ptr->value()));
            if(!t.has_value()){/*TODO: Error*/}
            auto tt = t.value_or(std::string_view(""));
            std::string_view sv = std::holds_alternative<std::string>(tt)?std::get<std::string>(tt):std::get<std::string_view>(tt);
            VS_XML_NS::print(out,"<!--{}-->",sv);
        }
        else{
            VS_XML_NS::print(out,"<!--{}-->",rsv(*ptr->value()));
        }
    }
    else if(ptr->type()==type_t::TEXT){
        if(!configs.raw_strings){
            auto t = serialize::to_xml_text(rsv(*ptr->value()));
            if(!t.has_value()){/*TODO: Error*/}
            auto tt = t.value_or(std::string_view(""));
            std::string_view  sv = std::holds_alternative<std::string>(tt)?std::get<std::string>(tt):std::get<std::string_view>(tt);
            VS_XML_NS::print(out,"{}",sv);
        }
        else{
            VS_XML_NS::print(out,"{}",rsv(*ptr->value()));
        }
    }
    else if(ptr->type()==type_t::PROC){
        if(!configs.raw_strings){
            auto t = serialize::to_xml_proc(rsv(*ptr->value()));
            if(!t.has_value()){/*TODO: Error*/}
            auto tt = t.value_or(std::string_view(""));
            std::string_view  sv = std::holds_alternative<std::string>(tt)?std::get<std::string>(tt):std::get<std::string_view>(tt);
            VS_XML_NS::print(out,"<?{}?>",sv);
        }
        else{
            VS_XML_NS::print(out,"<?{}?>",rsv(*ptr->value()));
        }
    }
    else if(ptr->type()==type_t::MARKER){
        //Skip, marker points are not XML, they are only internally used.
        //or emit something in a special namespace? not sure
    }
    else{return false;}
    return true;
};

const TreeRaw TreeRaw::slice(const element_t* ref) const{
    //TODO: Move to std::expected
    xml_assert((uint8_t*)ref>=(uint8_t*)buffer.data() && (uint8_t*)ref<(uint8_t*)buffer.data()+buffer.size(), "out of bounds node pointer");
    xml_assert(ref->type()==type_t::ELEMENT, "cannot slice something which is not a node");

    if(ref==nullptr){
        xml_assert(root().type()==type_t::ELEMENT);
        ref=(const element_t*)&root();
    }

    std::span<uint8_t> tmp = {( uint8_t*)ref,ref->_size};
    return TreeRaw(configs,tmp,this->symbols);
};

TreeRaw TreeRaw::clone(const element_t* ref, bool reduce) const{
    xml_assert(false, "Not implemented, a change of store will be needed");
    exit(1);
    /*
    //TODO: Move to std::expected
    xml_assert((uint8_t*)ref>=(uint8_t*)buffer.data() && (uint8_t*)ref<(uint8_t*)buffer.data()+buffer.size(), "out of bounds node pointer");
    xml_assert(ref->type()==type_t::ELEMENT, "cannot clone something which is not a node");

    if(ref==nullptr){
        xml_assert(root().type()==type_t::ELEMENT);
        ref=(const element_t*)&root();
    }

    std::vector<uint8_t> buffer;
    std::vector<uint8_t> symbols;
    buffer.resize(ref->_size);
    memcpy((void*)buffer.data(),ref,ref->_size);

    if(reduce==false){
        //symbols.assign_range(this->symbols_i);
        symbols.assign(this->symbols_i.begin(),this->symbols_i.end());
    }
    else{
        //TODO: create a new symbols vector, only providing what is needed.
        //symbols.assign_range(this->symbols_i);
        symbols.assign(this->symbols_i.begin(),this->symbols_i.end());
    }

    return TreeRaw(configs,std::move(buffer),std::move(symbols));
    */
}


bool TreeRaw::save_binary(std::ostream& out)const{
    if(configs.symbols==builder_config_t::EXTERN_ABS)return false; //Symbols not relocatable.

    binary_header_t header{};
    header.configs = configs;
    if(header.configs.symbols==builder_config_t::EXTERN_REL)header.configs.symbols=builder_config_t::OWNED; //Symbols are copied even if the where shared, so they are now owned.

    size_t align_symbols = (header.size()+symbols.size_bytes()%16==0)?0:(16-(header.size()+symbols.size_bytes())%16);
    header.length_of_symbols = symbols.size_bytes();
    binary_header_t::section_t section = {{0,0},0,buffer.size_bytes()};
    out.write((const char*)&header, sizeof(header));
    out.write((const char*)&section, sizeof(binary_header_t::section_t));
    out.write((const char*)symbols.data(), symbols.size_bytes());
    if(align_symbols!=0){
        char tmp[16]{};
        out.write(tmp, align_symbols);
    }
    out.write((const char*)buffer.data(), buffer.size_bytes());
    out.flush();
    return true;
}

std::expected<TreeRaw, TreeRaw::from_binary_error_t> TreeRaw::from_binary(std::span<uint8_t> region){
    const binary_header_t& header = *(const binary_header_t*)region.data();

    if(region.size_bytes() < header.size())
        return std::unexpected(from_binary_error_t{from_binary_error_t::HeaderTooSmall});
        
    if(std::memcmp(header.magic, "$XML", 4) != 0)
        return std::unexpected(from_binary_error_t{from_binary_error_t::MagicMismatch});
    
    if(header.format_major != format_major)
        return std::unexpected(from_binary_error_t{from_binary_error_t::MajorVersionMismatch});
    
    if(header.format_minor > format_minor)
        return std::unexpected(from_binary_error_t{from_binary_error_t::MinorVersionTooHigh});
    
    if(header.docs_count != 1)
        return std::unexpected(from_binary_error_t{from_binary_error_t::TooManyDocs});

    if  (
            header.size__delta_ptr!=sizeof(delta_ptr_t) || 
            header.size__xml_count!=sizeof(xml_count_t) ||
            header.size__xml_enum_size!=sizeof(xml_enum_size_t) ||
            header.size__xml_size!=sizeof(xml_size_t)
        ) return std::unexpected(from_binary_error_t{from_binary_error_t::TypeMismatch});

    auto endianess = std::endian::native==std::endian::little?binary_header_t::endianess_t::LITTLE:binary_header_t::endianess_t::BIG;
    if(header.endianess!=endianess) return std::unexpected(from_binary_error_t{from_binary_error_t::TypeMismatch});

    //TODO: Restore bounds checks (?) on sections?

    return TreeRaw(header.configs,
        std::span<uint8_t>{region.data()+header.start_data()+header.region(0).base, header.region(0).length},
        std::span<uint8_t>{region.data()+header.size(), header.length_of_symbols}
    );
}


std::expected<const TreeRaw, TreeRaw::from_binary_error_t>  TreeRaw::from_binary(std::span<const uint8_t> region){
    return from_binary(std::span<uint8_t>{(uint8_t*)region.data(),(uint8_t*)region.data()+region.size_bytes()});
}

std::string_view TreeRaw::from_binary_error_t::msg() {
    switch(code) {
        case OK:                  return "OK";
        case HeaderTooSmall:      return "Header of loaded file not matching minimum size";
        case MagicMismatch:       return "Header of loaded file not matching the format";
        case MajorVersionMismatch:return "This binary was generated in a different major revision of the format.";
        case MinorVersionTooHigh: return "This binary was generated in a minor released after this build.";
        case TruncatedSpan:       return "Truncated span for the loaded file";
        case TreeOutOfBounds:     return "Tree for loaded file is out of bounds";
        case SymbolsOutOfBounds:  return "Symbol table for loaded file is out of bounds";
        case TooManyDocs:         return "Too many documents in the table";
        case TypeMismatch:        return "Mismatch of types between the compiled library and the binary";
        default:                  return "Unknown error";
    }
}

wrp::base_t<unknown_t> Tree::root() const{return {*this, &TreeRaw::root()};}

//TODO: these four functions and the two for iterators in *-impl.cpp are pretty much the same. I hate I need to copy/paste so much code around.

void TreeRaw::visit(const unknown_t* node, bool(*fn)(const unknown_t*)){
    while(true){
        if(node==nullptr)break;
        
        bool children_visited = !fn(node);
        for(;;){
            if(node->has_children() && !children_visited){
                auto [l,r] =*node->children_range();
                node=l;
                children_visited = false;
                break;
            }
            if(node->has_next()){
                node=node->next();
                children_visited = false;
                break;
            }
            if(node->has_parent()){
                node = (const unknown_t*)node->parent();
                children_visited = true;
            }
            else{
                node = nullptr;
                break;
            }
        }   
    }
}

void TreeRaw::visit(const unknown_t* node, std::function<bool(const unknown_t*)>&& fn){
    while(true){
        if(node==nullptr)break;
        
        bool children_visited = !fn(node);
        for(;;){
            if(node->has_children() && !children_visited){
                auto [l,r] =*node->children_range();
                node=l;
                children_visited = false;
                break;
            }
            if(node->has_next()){
                node=node->next();
                children_visited = false;
                break;
            }
            if(node->has_parent()){
                node = (const unknown_t*)node->parent();
                children_visited = true;
            }
            else{
                node = nullptr;
                break;
            }
        }
    }
}


void Tree::visit(wrp::base_t<unknown_t> node, bool(*fn)(wrp::base_t<unknown_t>)){
    while(true){
        if(node.ptr==nullptr)break;
        
        bool children_visited = !fn(node);
        for(;;){
            if(node.ptr->has_children() && !children_visited){
                auto [l,r] =*node.children_range();
                node=l;
                children_visited = false;
                break;
            }
            if(node.ptr->has_next()){
                node=node.next();
                children_visited = false;
                break;
            }
            if(node.ptr->has_parent()){
                node.ptr = (const unknown_t*) node.parent().ptr;
                children_visited = true;
            }
            else{
                node.ptr = nullptr;
                break;
            }
        }
    }
}

void Tree::visit(wrp::base_t<unknown_t> node, std::function<bool(wrp::base_t<unknown_t>)>&& fn){
    while(true){
        if(node.ptr==nullptr)break;
        
        bool children_visited = !fn(node);
        for(;;){
            if(node.ptr->has_children() && !children_visited){
                auto [l,r] =*node.children_range();
                node=l;
                children_visited = false;
                break;
            }
            if(node.ptr->has_next()){
                node=node.next();
                children_visited = false;
                break;
            }
            if(node.ptr->has_parent()){
                node.ptr = (const unknown_t*) node.parent().ptr;
                children_visited = true;
            }
            else{
                node.ptr = nullptr;
                break;
            }
        }
    }
}

}