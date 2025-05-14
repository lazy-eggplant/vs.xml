#include <cstring>
#include <algorithm>
#include <string_view>

#include <vs-xml/commons.hpp>
#include <vs-xml/tree.hpp>
#include <vs-xml/impl.hpp>
#include <vs-xml/wrp-impl.hpp>
#include <vs-xml/serializer.hpp>

namespace VS_XML_NS{

std::function<bool(const unknown_t&, const unknown_t&)> TreeRaw::def_order_node() const{
    throw "Not implemented";
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
    if(ref==nullptr)ref=&root();

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
            out << std::format("<{}{}{}", rsv(*ptr->ns()), rsv(*ptr->ns())==""?"":":", rsv(*ptr->name()));
            for(auto& i : ptr->attrs()){
                if(!configs.raw_strings){
                    auto t = serialize::to_xml_attr_2(rsv(*i.value()));
                    if(!t.has_value()){/*TODO: Error*/}
                    auto tt = t.value_or(std::string_view(""));
                    std::string_view  sv = std::holds_alternative<std::string>(tt)?std::get<std::string>(tt):std::get<std::string_view>(tt);
                    out << std::format(" {}{}{}=\"{}\"", rsv(*i.ns()), rsv(*i.ns())==""?"":":", rsv(*i.name()), sv);
                }
                else{
                    out << std::format(" {}{}{}=\"{}\"", rsv(*i.ns()), rsv(*i.ns())==""?"":":", rsv(*i.name()), rsv(*i.value()));
                }
            }
            out << "/>";
        }
        else{
            out << std::format("<{}{}{}", rsv(*ptr->ns()), rsv(*ptr->ns())==""?"":":", rsv(*ptr->name()));
            for(auto& i : ptr->attrs()){
                if(!configs.raw_strings){
                    auto t = serialize::to_xml_attr_2(rsv(*i.value()));
                    if(!t.has_value()){/*TODO: Error*/}
                    auto tt = t.value_or(std::string_view(""));
                    std::string_view  sv = std::holds_alternative<std::string>(tt)?std::get<std::string>(tt):std::get<std::string_view>(tt);
                    out << std::format(" {}{}{}=\"{}\"", rsv(*i.ns()), rsv(*i.ns())==""?"":":", rsv(*i.name()), sv);
                }
                else{
                    out << std::format(" {}{}{}=\"{}\"", rsv(*i.ns()), rsv(*i.ns())==""?"":":", rsv(*i.name()), rsv(*i.value()));
                }
            }
            out << ">";
            for(auto& i : ptr->children()){
                print_h(out,cfg,&i);
            }
            out << std::format("</{}{}{}>", rsv(*ptr->ns()), rsv(*ptr->ns())==""?"":":", rsv(*ptr->name()));
        }
    }
    else if(ptr->type()==type_t::CDATA){
        if(!configs.raw_strings){
            auto t = serialize::to_xml_cdata(rsv(*ptr->value()));
            if(!t.has_value()){/*TODO: Error*/}
            auto tt = t.value_or(std::string_view(""));
            std::string_view sv = std::holds_alternative<std::string>(tt)?std::get<std::string>(tt):std::get<std::string_view>(tt);
            out << std::format("<![CDATA[{}]]>",sv);
        }
        else{
            out << std::format("<![CDATA[{}]]>",rsv(*ptr->value()));
        }
    }
    else if(ptr->type()==type_t::COMMENT){
        if(!configs.raw_strings){
            auto t = serialize::to_xml_comment(rsv(*ptr->value()));
            if(!t.has_value()){/*TODO: Error*/}
            auto tt = t.value_or(std::string_view(""));
            std::string_view sv = std::holds_alternative<std::string>(tt)?std::get<std::string>(tt):std::get<std::string_view>(tt);
            out << std::format("<!--{}-->",sv);
        }
        else{
            out << std::format("<!--{}-->",rsv(*ptr->value()));
        }
    }
    else if(ptr->type()==type_t::TEXT){
        if(!configs.raw_strings){
            auto t = serialize::to_xml_text(rsv(*ptr->value()));
            if(!t.has_value()){/*TODO: Error*/}
            auto tt = t.value_or(std::string_view(""));
            std::string_view  sv = std::holds_alternative<std::string>(tt)?std::get<std::string>(tt):std::get<std::string_view>(tt);
            out << std::format("{}",sv);
        }
        else{
            out << std::format("{}",rsv(*ptr->value()));
        }
    }
    else if(ptr->type()==type_t::PROC){
        if(!configs.raw_strings){
            auto t = serialize::to_xml_proc(rsv(*ptr->value()));
            if(!t.has_value()){/*TODO: Error*/}
            auto tt = t.value_or(std::string_view(""));
            std::string_view  sv = std::holds_alternative<std::string>(tt)?std::get<std::string>(tt):std::get<std::string_view>(tt);
            out << std::format("<?{}?>",sv);
        }
        else{
            out << std::format("<?{}?>",rsv(*ptr->value()));
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
    xml_assert((uint8_t*)ref>=(uint8_t*)buffer.data() && (uint8_t*)ref<(uint8_t*)buffer.data()+buffer.size(), "out of bounds node pointer");
    xml_assert(ref->type()==type_t::ELEMENT, "cannot slice something which is not a node");

    if(ref==nullptr)ref=&root();

    std::span<uint8_t> tmp = {( uint8_t*)ref,ref->_size};
    return TreeRaw(configs,tmp,this->symbols);
};

TreeRaw TreeRaw::clone(const element_t* ref, bool reduce) const{
    xml_assert((uint8_t*)ref>=(uint8_t*)buffer.data() && (uint8_t*)ref<(uint8_t*)buffer.data()+buffer.size(), "out of bounds node pointer");
    xml_assert(ref->type()==type_t::ELEMENT, "cannot clone something which is not a node");

    if(ref==nullptr)ref=&root();

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
}


bool TreeRaw::save_binary(std::ostream& out)const{
    //Symbols not relocatable.
    if(symbols.data()==nullptr)return false;

    serialized_header_t header;
    header.binformat_rev = 0;
    header.configs = configs;
    header.offset_symbols = buffer.size_bytes();
    header.offset_end = buffer.size_bytes()+symbols.size_bytes();
    out.write((const char*) &header, sizeof(header));
    out.write((const char*)buffer.data(), buffer.size_bytes());
    out.write((const char*)symbols.data(), symbols.size_bytes());
    out.flush();
    return true;
}

//TODO: check if there are more checks between the const and mutable versions based on the configuration bits
TreeRaw TreeRaw::from_binary(std::span<uint8_t> region){
    xml_assert(region.size_bytes()>sizeof(serialized_header_t),"Header of loaded file not matching minimum size");
    TreeRaw::serialized_header_t header;
    memcpy((void*)&header,region.data(),sizeof(serialized_header_t));
    xml_assert(memcmp(header.magic,"$XML",4)==0,"Header of loaded file not matching the format");
    xml_assert(region.size_bytes()>=sizeof(serialized_header_t)+header.offset_end, "Truncated span for the loaded file");
    xml_assert(header.offset_symbols<=header.offset_end, "Symbol table for loaded file is out of bounds");

    return TreeRaw(header.configs,
        std::span<uint8_t>{region.data()+sizeof(header), region.data()+sizeof(header)+header.offset_symbols},
        std::span<uint8_t>{region.data()+sizeof(header)+header.offset_symbols, region.data()+sizeof(header)+header.offset_end}
    );
}

const TreeRaw TreeRaw::from_binary(std::string_view region){
    xml_assert(region.size()>sizeof(serialized_header_t),"Header of loaded file not matching minimum size");
    TreeRaw::serialized_header_t header;
    memcpy((void*)&header,region.data(),sizeof(serialized_header_t));
    xml_assert(memcmp(header.magic,"$XML",4)==0,"Header of loaded file not matching the format");
    xml_assert(region.size()>=sizeof(serialized_header_t)+header.offset_end, "Truncated span for the loaded file");
    xml_assert(header.offset_symbols<=header.offset_end, "Symbol table for loaded file is out of bounds");

    return TreeRaw(header.configs,
        std::string_view{region.data()+sizeof(header), region.data()+sizeof(header)+header.offset_symbols},
        std::string_view{region.data()+sizeof(header)+header.offset_symbols, region.data()+sizeof(header)+header.offset_end}
    );
}

wrp::base_t<element_t> Tree::root() const{return {*this, &TreeRaw::root()};}

}