#include <cstring>
#include <algorithm>
#include <string_view>

#include <vs-xml/commons.hpp>
#include <vs-xml/tree.hpp>
#include <vs-xml/node.hpp>
#include <vs-xml/wrp-node.hpp>
#include <vs-xml/serializer.hpp>

#include <vs-xml/fwd/print.hpp>
#include <vs-xml/private/visit.hpp>
#include <vs-xml/private/wrp-visit.hpp>

namespace VS_XML_NS{

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

std::function<bool(const unknown_t&, const unknown_t&)> TreeRaw::def_order_node() const{
    return [this](const unknown_t& a, const unknown_t& b){
        if(a.type()!=b.type())return (int)a.type()<(int)b.type();
        auto an = a.name(), bn = b.name();
        if(an.has_value() && bn.has_value()){
            auto va = rsv(*an), vb = rsv(*bn);
            if(va<vb)return true;
            else if(va>vb)return false;
        }
        auto av = a.value(), bv = b.value();
        if(av.has_value() && bv.has_value()){
            auto va = rsv(*av), vb = rsv(*bv);
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
    return reorder_h(fn,ref,recursive);
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

void TreeRaw::print_node(std::ostream& out, const print_cfg_t& cfg, const unknown_t* ptr, int depth) const{
    const char* nl = cfg.use_lf ? "\n" : "\r\n";

    auto indent = [&](int level){
        if(cfg.use_tabs){
            for(int i=0;i<level;i++)out.put('\t');
        } else {
            for(int i=0;i<level*cfg.spaces_in_tab;i++)out.put(' ');
        }
    };

    auto write_attrs = [&](const unknown_t* node){
        const char quote = cfg.use_quote ? '"' : '\'';
        for(auto& i : node->attrs()){
            out.put(' ');
            auto ns = rsv(*i.ns());
            if(!ns.empty())out << ns << ':';
            out << rsv(*i.name()) << '=' << quote;
            if(configs.raw_strings){
                out << rsv(*i.value());
            } else {
                auto t = (quote=='"') ? serialize::to_xml_attr_2(rsv(*i.value())) : serialize::to_xml_attr_1(rsv(*i.value()));
                if(t.has_value()){
                    auto& tt = *t;
                    if(std::holds_alternative<std::string>(tt))out << std::get<std::string>(tt);
                    else out << std::get<std::string_view>(tt);
                }
            }
            out << quote;
        }
    };

    switch(ptr->type()){
        case type_t::ELEMENT: {
            auto range = *ptr->children_range();
            const unknown_t* first = range.first;
            const unknown_t* last = range.second;
            const bool has_children = (first != last);
            bool mixed = false;
            for(const unknown_t* c = first; c != last; c = c->next()){
                if(c->type()==type_t::TEXT || c->type()==type_t::CDATA){mixed = true; break;}
            }

            auto ns = rsv(*ptr->ns());
            auto name = rsv(*ptr->name());
            out.put('<');
            if(!ns.empty())out << ns << ':';
            out << name;
            write_attrs(ptr);

            if(!has_children){
                out << "/>";
                return;
            }
            out.put('>');
            if(mixed){
                for(const unknown_t* c = first; c != last; c = c->next()) print_node(out,cfg,c,depth);
            } else {
                for(const unknown_t* c = first; c != last; c = c->next()){
                    out << nl;
                    indent(depth+1);
                    print_node(out,cfg,c,depth+1);
                }
                out << nl;
                indent(depth);
            }
            out << "</";
            if(!ns.empty())out << ns << ':';
            out << name << '>';
            break;
        }
        case type_t::TEXT: {
            if(configs.raw_strings){ out << rsv(*ptr->value()); }
            else {
                auto t = serialize::to_xml_text(rsv(*ptr->value()));
                auto tt = t.value_or(std::string_view(""));
                if(std::holds_alternative<std::string>(tt))out << std::get<std::string>(tt);
                else out << std::get<std::string_view>(tt);
            }
            break;
        }
        case type_t::CDATA: {
            auto v = rsv(*ptr->value());
            if(configs.raw_strings){ out << "<![CDATA[" << v << "]]>"; }
            else if(auto t = serialize::to_xml_cdata(v); t.has_value()){
                auto tt = *t;
                out << "<![CDATA[";
                if(std::holds_alternative<std::string>(tt))out << std::get<std::string>(tt);
                else out << std::get<std::string_view>(tt);
                out << "]]>";
            }
            break;
        }
        case type_t::COMMENT: {
            if(configs.raw_strings){ out << "<!--" << rsv(*ptr->value()) << "-->"; }
            else {
                auto t = serialize::to_xml_comment(rsv(*ptr->value()));
                auto tt = t.value_or(std::string_view(""));
                out << "<!--";
                if(std::holds_alternative<std::string>(tt))out << std::get<std::string>(tt);
                else out << std::get<std::string_view>(tt);
                out << "-->";
            }
            break;
        }
        case type_t::PROC: {
            if(configs.raw_strings){ out << "<?" << rsv(*ptr->value()) << "?>"; }
            else {
                auto t = serialize::to_xml_proc(rsv(*ptr->value()));
                auto tt = t.value_or(std::string_view(""));
                out << "<?";
                if(std::holds_alternative<std::string>(tt))out << std::get<std::string>(tt);
                else out << std::get<std::string_view>(tt);
                out << "?>";
            }
            break;
        }
        case type_t::MARKER:
            break;
        default:
            break;
    }
}


const TreeRaw TreeRaw::slice(const element_t* ref) const{
    //TODO: Move to std::expected
    xml_assert((uint8_t*)ref>=(uint8_t*)buffer.data() && (uint8_t*)ref<(uint8_t*)buffer.data()+buffer.size(), "out of bounds node pointer");
    xml_assert(ref->type()==type_t::ELEMENT, "cannot slice something which is not a node");

    if(ref==nullptr){
        xml_assert(root().type()==type_t::ELEMENT);
        ref=(const element_t*)&root();
    }

    std::span<uint8_t> tmp = {( uint8_t*)ref,(size_t)ref->_next};
    return TreeRaw(configs,tmp,this->symbols);
};


bool TreeRaw::save_binary(std::ostream& out)const{
    if(configs.symbols==builder_config_t::EXTERN_ABS)return false; //Symbols not relocatable.

    binary_header_t header{};
    header.configs = configs;
    if(header.configs.symbols==builder_config_t::EXTERN_REL)header.configs.symbols=builder_config_t::OWNED; //Symbols are copied even if the where shared, so they are now owned.

    header.docs_count = 1;
    header.length_of_symbols = symbols.size_bytes();
    const size_t total = header.size() + symbols.size_bytes();
    const size_t align_symbols = (total%16==0)?0:(16-total%16);
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
    // Never dereference the header before proving the region can hold it.
    if(region.size_bytes() < sizeof(binary_header_t))
        return std::unexpected(from_binary_error_t{from_binary_error_t::HeaderTooSmall});

    const binary_header_t& header = *(const binary_header_t*)region.data();

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

    const size_t region_size = region.size_bytes();
    if(header.size() > region_size)
        return std::unexpected(from_binary_error_t{from_binary_error_t::HeaderTooSmall});

    const size_t after_header = region_size - header.size();
    if(header.length_of_symbols > after_header)
        return std::unexpected(from_binary_error_t{from_binary_error_t::SymbolsOutOfBounds});

    const size_t data_start = header.start_data();
    if(data_start > region_size)
        return std::unexpected(from_binary_error_t{from_binary_error_t::TruncatedSpan});

    // In-place node structs are reinterpreted from this address, so it must be aligned.
    if(reinterpret_cast<uintptr_t>(region.data()+data_start) % alignof(std::max_align_t) != 0)
        return std::unexpected(from_binary_error_t{from_binary_error_t::MisalignedRegion});

    // The single document must fit inside the data region.
    const binary_header_t::section_t sec = header.region(0);
    if(sec.base < 0 || (size_t)sec.base > region_size - data_start)
        return std::unexpected(from_binary_error_t{from_binary_error_t::TreeOutOfBounds});
    if(sec.length > region_size - data_start - (size_t)sec.base)
        return std::unexpected(from_binary_error_t{from_binary_error_t::TreeOutOfBounds});

    return TreeRaw(header.configs,
        std::span<uint8_t>{region.data()+data_start+(size_t)sec.base, (size_t)sec.length},
        std::span<uint8_t>{region.data()+header.size(), (size_t)header.length_of_symbols}
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
        case MisalignedRegion:    return "The data region is not suitably aligned for the in-place nodes";
        default:                  return "Unknown error";
    }
}

[[nodiscard]] wrp::base_t<unknown_t> Tree::root() const{return {*this, &TreeRaw::root()};}

void TreeRaw::visit(const unknown_t* node, bool(*test)(const unknown_t*, void* ctx), void(*before)(const unknown_t*, void* ctx), void(*after)(const unknown_t*, void* ctx), void* ctx){
    VS_XML_NS::visit<>(node,test,before,after,ctx);
}

void TreeRaw::visit(const unknown_t* node, std::function<bool(const unknown_t*)>&& test, std::function<void(const unknown_t*)>&& before, std::function<void(const unknown_t*)>&& after){
    VS_XML_NS::visit<>(node,test,before,after);
}

void Tree::visit(wrp::base_t<unknown_t> node, bool(*test)(wrp::base_t<unknown_t>, void* ctx), void(*before)(wrp::base_t<unknown_t>, void* ctx), void(*after)(wrp::base_t<unknown_t>, void* ctx), void* ctx){
    VS_XML_NS::wrp::visit<>(node,test,before,after,ctx);
}

void Tree::visit(wrp::base_t<unknown_t> node, std::function<bool(wrp::base_t<unknown_t>)>&& test, std::function<void(wrp::base_t<unknown_t>)>&& before, std::function<void(wrp::base_t<unknown_t>)>&& after){
    VS_XML_NS::wrp::visit<>(node,test,before,after);
}

bool TreeRaw::print(std::ostream& out, const print_cfg_t& cfg, const unknown_t* node)const{
    if(node==nullptr){
        if(buffer.empty())return true;
        node = (const unknown_t*)&root();
    }
    print_node(out,cfg,node,0);
    return (bool)out;
}

bool TreeRaw::print_fast(std::ostream& out, const print_cfg_t& cfg, const unknown_t* node)const{
    if(node==nullptr){
        if(buffer.empty())return true;
        node = (const unknown_t*)&root();
    }
    print_node(out,cfg,node,0);
    return (bool)out;
}

}