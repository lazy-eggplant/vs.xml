#include <vs-xml/tree.hpp>
#include <vs-xml/impl.hpp>

namespace xml{

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