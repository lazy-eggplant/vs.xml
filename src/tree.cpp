#include "vs-xml/commons.hpp"
#include <cstring>
#include <vs-xml/tree.hpp>
#include <vs-xml/impl.hpp>

namespace xml{

std::function<bool(const unknown_t&, const unknown_t&)> Tree::def_order_node;
std::function<bool(const attr_t&, const attr_t&)> Tree::def_order_attrs() {
    return [this](const attr_t& a, const attr_t& b){
        if(rsv(*a.ns())<rsv(*b.ns()))return true;
        if(rsv(*a.name())<rsv(*b.name()))return true;
        return false;
    };
}

bool Tree::reorder(const std::function<bool(const attr_t&, const attr_t&)>& fn, const node_t* ref,  bool recursive){
    static std::vector<xml_count_t> swap = {};

    if(ref==nullptr)ref=&this->root;
    if(ref->type()!=type_t::NODE)return false;

    //if(swap.size()>64)swap.clear();   //To avoid it growing too much for single spikes of cases with many attributes. This value can be safely tuned.
    if(swap.size()<ref->attrs_count)swap.resize(ref->attrs_count);

    for(size_t i = 0; i<ref->attrs_count; i++)swap[i]=i;

    std::sort(swap.begin(),swap.begin()+ref->attrs_count,[&](xml_count_t a, xml_count_t b){return fn(ref->get_attr(a),ref->get_attr(b));});

    //for(size_t i = 0; i<ref->attrs_count; i++)printf("%lu ",swap[i]);
    //printf("\n");

    for(size_t i = 0; i<ref->attrs_count; i++){
        printf("%lu\n",(*ref->get_attr(i).name()).base);

        attr_t tmp = ref->get_attr(i);
        ref->get_attr(i) =ref->get_attr(swap[i]);
        ref->get_attr(swap[i]) = tmp;

        printf("%lu\n",(*ref->get_attr(i).name()).base);
    }

    if(recursive){
        for(auto &i: ref->children_fwd()){
            if(i.type()==type_t::NODE)
            reorder(fn,&(const node_t&)i,recursive);
        }
    }

    return true;
};


//TODO: at some point, convert it not to be recursive.
bool Tree::print_h(std::ostream& out, const print_cfg_t& cfg, const unknown_t* ptr) const{
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
}