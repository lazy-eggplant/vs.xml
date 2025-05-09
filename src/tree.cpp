#include <cstring>
#include <vs-xml/commons.hpp>
#include <vs-xml/tree.hpp>
#include <vs-xml/impl.hpp>
#include <vs-xml/serializer.hpp>

namespace VS_XML_NS{

std::function<bool(const unknown_t&, const unknown_t&)> Tree::def_order_node(){
    throw "Not implemented";
}

std::function<bool(const attr_t&, const attr_t&)> Tree::def_order_attrs() {
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

bool Tree::reorder(const std::function<bool(const attr_t&, const attr_t&)>& fn, const node_t* ref,  bool recursive){
    if(ref==nullptr)ref=&root();

    xml_assert((uint8_t*)ref>=(uint8_t*)buffer.data() && (uint8_t*)ref<(uint8_t*)buffer.data()+buffer.size());
    xml_assert(ref->type()==type_t::NODE);
    return reorder_h(def_order_attrs(),ref,recursive);
}

bool Tree::reorder_h(const std::function<bool(const attr_t&, const attr_t&)>& fn, const node_t* ref,  bool recursive){
    //Speed could be improved by using an intermediate swapping function, but attr_t elements are small enough that it might not be worthed.
    //TODO: at some point, convert it not to be recursive.

    if(ref->type()!=type_t::NODE)return false;

    std::sort((attr_t*)ref->_attrs,(attr_t*)ref->_attrs+ref->attrs_count,fn);

    if(recursive){
        for(auto &i: ref->children_fwd()){
            if(i.type()==type_t::NODE)
            reorder_h(fn,&(const node_t&)i,recursive);
        }
    }

    return true;
};


bool Tree::print_h(std::ostream& out, const print_cfg_t& cfg, const unknown_t* ptr) const{
    //TODO: at some point, convert it not to be recursive.

    if(ptr->type()==type_t::NODE){
        if(ptr->children()->first==ptr->children()->second){
            out << std::format("<{}{}{}", rsv(*ptr->ns()), rsv(*ptr->ns())==""?"":":", rsv(*ptr->name()));
            for(auto& i : ptr->attrs_fwd()){
                auto t = serialize::to_xml_attr_2(rsv(*i.value()));
                if(!t.has_value()){/*TODO: Error*/}
                auto tt = t.value_or(std::string_view(""));
                std::string_view sv = std::holds_alternative<std::string>(tt)?std::get<std::string>(tt):std::get<std::string_view>(tt);
                out << std::format(" {}{}{}=\"{}\"", rsv(*i.ns()), rsv(*i.ns())==""?"":":", rsv(*i.name()), sv);
            }
            out << "/>";
        }
        else{
            out << std::format("<{}{}{}", rsv(*ptr->ns()), rsv(*ptr->ns())==""?"":":", rsv(*ptr->name()));
            for(auto& i : ptr->attrs_fwd()){
                auto t = serialize::to_xml_attr_2(rsv(*i.value()));
                if(!t.has_value()){/*TODO: Error*/}
                auto tt = t.value_or(std::string_view(""));
                std::string_view sv = std::holds_alternative<std::string>(tt)?std::get<std::string>(tt):std::get<std::string_view>(tt);
                out << std::format(" {}{}{}=\"{}\"", rsv(*i.ns()), rsv(*i.ns())==""?"":":", rsv(*i.name()), sv);
            }
            out << ">";
            for(auto& i : ptr->children_fwd()){
                print_h(out,cfg,&i);
            }
            out << std::format("</{}{}{}>", rsv(*ptr->ns()), rsv(*ptr->ns())==""?"":":", rsv(*ptr->name()));
        }
    }
    else if(ptr->type()==type_t::CDATA){
        auto t = serialize::to_xml_cdata(rsv(*ptr->value()));
        if(!t.has_value()){/*TODO: Error*/}
        auto tt = t.value_or(std::string_view(""));
        std::string_view sv = std::holds_alternative<std::string>(tt)?std::get<std::string>(tt):std::get<std::string_view>(tt);
        out << std::format("<![CDATA[{}]]>",sv);
    }
    else if(ptr->type()==type_t::COMMENT){
        auto t = serialize::to_xml_comment(rsv(*ptr->value()));
        if(!t.has_value()){/*TODO: Error*/}
        auto tt = t.value_or(std::string_view(""));
        std::string_view sv = std::holds_alternative<std::string>(tt)?std::get<std::string>(tt):std::get<std::string_view>(tt);
        out << std::format("<!--{}-->",sv);
    }
    else if(ptr->type()==type_t::TEXT){
        auto t = serialize::to_xml_text(rsv(*ptr->value()));
        if(!t.has_value()){/*TODO: Error*/}
        auto tt = t.value_or(std::string_view(""));
        std::string_view sv = std::holds_alternative<std::string>(tt)?std::get<std::string>(tt):std::get<std::string_view>(tt);
        out << std::format("{}",sv);
    }
    else if(ptr->type()==type_t::PROC){
        auto t = serialize::to_xml_proc(rsv(*ptr->value()));
        if(!t.has_value()){/*TODO: Error*/}
        auto tt = t.value_or(std::string_view(""));
        std::string_view sv = std::holds_alternative<std::string>(tt)?std::get<std::string>(tt):std::get<std::string_view>(tt);
        out << std::format("<?{}?>",sv);
    }
    else if(ptr->type()==type_t::INJECT){
        //Skip, injection points are not XML, they are only internally used.
    }
    return false;
};

Tree Tree::clone(const node_t* ref, bool reduce) const{
    xml_assert((uint8_t*)ref>=(uint8_t*)buffer.data() && (uint8_t*)ref<(uint8_t*)buffer.data()+buffer.size());
    xml_assert(ref->type()==type_t::NODE);

    if(ref==nullptr)ref=&root();

    std::vector<uint8_t> buffer;
    std::vector<uint8_t> symbols;
    buffer.resize(ref->_size);
    memcpy((void*)buffer.data(),ref,ref->_size);

    if(reduce==false){
       symbols = this->symbols;
    }
    else{
        //TODO: create a new symbols vector, only providing what is needed.
        symbols = this->symbols;

    }

    return Tree(std::move(buffer),std::move(symbols));
}

}