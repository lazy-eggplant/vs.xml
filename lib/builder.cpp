#include <vs-xml/commons.hpp>
#include <vs-xml/builder.hpp>
#include <vs-xml/impl.hpp>

namespace VS_XML_NS{

namespace details{

template<typename T>
BuilderBase::error_t BuilderBase::leaf(std::string_view value){
    if(open==false)return error_t::TREE_CLOSED;
    attribute_block=false;

    buffer.resize(buffer.size()+sizeof(T));

    auto& old_ctx = stack.top();

    unknown_t* prev = old_ctx.second!=-1?(unknown_t*)(buffer.data()+old_ctx.second):nullptr;

    T* tmp_node = new ((element_t*) & (uint8_t&) *( buffer.end()-sizeof(T) )) T(symbols.data(),value);

    if(prev!=nullptr){
        prev->set_next((unknown_t*)tmp_node);
        tmp_node->set_prev(prev);
    }
    old_ctx.second = (uint8_t*)tmp_node-buffer.data();

    return error_t::OK;
}

template BuilderBase::error_t BuilderBase::leaf<comment_t>(std::string_view value);
template BuilderBase::error_t BuilderBase::leaf<cdata_t>(std::string_view value);
template BuilderBase::error_t BuilderBase::leaf<text_t>(std::string_view value);
template BuilderBase::error_t BuilderBase::leaf<proc_t>(std::string_view value);
template BuilderBase::error_t BuilderBase::leaf<marker_t>(std::string_view value);


BuilderBase::BuilderBase(){stack.push({0,-1});}

BuilderBase::error_t BuilderBase::begin(std::string_view name, std::string_view ns){
    if(open==false)return error_t::TREE_CLOSED;

    buffer.resize(buffer.size()+sizeof(element_t));

    auto& old_ctx = stack.top();

    element_t* parent = (element_t*)(buffer.data()+old_ctx.first);
    unknown_t* prev = old_ctx.second!=-1?(unknown_t*)(buffer.data()+old_ctx.second):nullptr;

    //Emplace node
    element_t* tmp_node = new ((element_t*) & (uint8_t&) *( buffer.end()-sizeof(element_t) )) element_t(symbols.data(),parent,ns,name);

    if(prev!=nullptr){
        prev->set_next((unknown_t*)tmp_node);
        tmp_node->set_prev(prev);
    }
    old_ctx.second = (uint8_t*)tmp_node-buffer.data();

    stack.push({((uint8_t*)tmp_node-(uint8_t*)buffer.data()),-1});
    attribute_block=true;

    return error_t::OK;
}

BuilderBase::error_t BuilderBase::end(){
    if(open==false)return error_t::TREE_CLOSED;
    if(stack.size()<=1)return error_t::STACK_EMPTY;

    attribute_block=false;

    auto& ctx = stack.top();
    element_t* parent = (element_t*)(buffer.data()+ctx.first);
    parent->_size=buffer.size()-ctx.first;

    stack.pop();

    return error_t::OK;
}

BuilderBase::error_t BuilderBase::attr(std::string_view name, std::string_view value, std::string_view ns){
    if(open==false)return error_t::TREE_CLOSED;
    if(attribute_block==false)return error_t::TREE_ATTR_CLOSED;

    buffer.resize(buffer.size()+sizeof(attr_t));

    auto& old_ctx = stack.top();

    element_t* parent = (element_t*)(buffer.data()+old_ctx.first);

    //Emplace node
    new ((attr_t*) & (uint8_t&) *( buffer.end()-sizeof(attr_t) )) attr_t((void*)symbols.data(),ns,name,value);

    parent->attrs_count++;

    return error_t::OK;
}

BuilderBase::error_t BuilderBase::close(){
    if(open==false)return error_t::TREE_CLOSED;
    open=false;
    if(stack.size()!=1)return error_t::MISFORMED;
    stack.pop();
    return error_t::OK;
}


sv BuilderImpl<builder_config_t::symbols_t::COMPRESS_ALL>::symbol(std::string_view s){
    if(s.length()==0)return {0,0};

    auto it = idx_symbols.find(sv(symbols.data(),s));

    if(it==idx_symbols.end()){
        symbols_i.insert(symbols_i.end(),s.begin(),s.end());
        symbols={(const char*)symbols_i.data(),(const char*)symbols_i.data()+symbols_i.size()};    //Keeep this updated so that when sv are used they are always relative.

        sv ret(symbols_i.size()-s.length(),s.length());
        auto it = idx_symbols.insert(ret); 
        if(it.second==false)throw std::runtime_error("Unable to insert symbol in symbol table");
        return *it.first;
    }
    else{
        return *it;
    }
}

sv BuilderImpl<builder_config_t::symbols_t::OWNED>::symbol(std::string_view s){
    if(s.length()==0)return {0,0};

    symbols_i.insert(symbols_i.end(),s.begin(),s.end());
    symbols={(const char*)symbols_i.data(),(const char*)symbols_i.data()+symbols_i.size()};    //Keeep this updated so that when sv are used they are always relative.

    sv ret(symbols_i.size()-s.length(),s.length());
    return ret;
}

//TODO: Add symbol2 for COMPRESS_ALL which does not compress it.

BuilderBase::error_t BuilderBase::inject(const TreeRaw& tree, const unknown_t* base, bool include_root){
    if(base==nullptr)base=(const unknown_t*)&tree.root();
    //If the symbol offset for tree and BuilderBase is the same, we are good and memcopy is possible.
    //If not, and the Builder has no symbols compression ongoing, the injection will be rejected.
    //If symbol compression is ongoing, the tree content will be "parsed" and added via fast bytecode operations.

    throw "Not implemented";
}

}
}