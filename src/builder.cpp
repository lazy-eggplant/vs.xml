#include <stdexcept>
#include <vs-xml/builder.hpp>
#include <vs-xml/impl.hpp>

namespace VS_XML_NS{

template<typename T>
Builder::error_t Builder::leaf(std::string_view value){
    if(open==false)return error_t::TREE_CLOSED;
    attribute_block=false;

    buffer.resize(buffer.size()+sizeof(T));

    auto& old_ctx = stack.top();

    unknown_t* prev = old_ctx.second!=-1?(unknown_t*)(buffer.data()+old_ctx.second):nullptr;

    T* tmp_node = new ((node_t*) & (uint8_t&) *( buffer.end()-sizeof(T) )) T(label_offset,value);

    if(prev!=nullptr){
        prev->set_next((unknown_t*)tmp_node);
        tmp_node->set_prev(prev);
    }
    old_ctx.second = (uint8_t*)tmp_node-buffer.data();

    return error_t::OK;
}

template Builder::error_t Builder::leaf<comment_t>(std::string_view value);
template Builder::error_t Builder::leaf<cdata_t>(std::string_view value);
template Builder::error_t Builder::leaf<text_t>(std::string_view value);
template Builder::error_t Builder::leaf<proc_t>(std::string_view value);
template Builder::error_t Builder::leaf<inject_t>(std::string_view value);

std::expected<WrpTree,Builder::error_t> Builder::close(std::vector<uint8_t>&& symbols){
    if(open==false)return std::unexpected(error_t::TREE_CLOSED);
    open=false;
    if(stack.size()!=1)return std::unexpected(error_t::MISFORMED);
    stack.pop();
    
    return WrpTree(Tree(std::move(buffer),std::move(symbols)));
}

Builder::Builder(config_t cfg):cfg(cfg){stack.push({0,-1});}

Builder::error_t Builder::begin(std::string_view name, std::string_view ns){
    if(open==false)return error_t::TREE_CLOSED;

    buffer.resize(buffer.size()+sizeof(node_t));

    auto& old_ctx = stack.top();

    node_t* parent = (node_t*)(buffer.data()+old_ctx.first);
    unknown_t* prev = old_ctx.second!=-1?(unknown_t*)(buffer.data()+old_ctx.second):nullptr;

    //Emplace node
    node_t* tmp_node = new ((node_t*) & (uint8_t&) *( buffer.end()-sizeof(node_t) )) node_t(label_offset,parent,ns,name);

    if(prev!=nullptr){
        prev->set_next((unknown_t*)tmp_node);
        tmp_node->set_prev(prev);
    }
    old_ctx.second = (uint8_t*)tmp_node-buffer.data();

    stack.push({((uint8_t*)tmp_node-(uint8_t*)buffer.data()),-1});
    attribute_block=true;

    return error_t::OK;
}

Builder::error_t Builder::end(){
    if(open==false)return error_t::TREE_CLOSED;
    if(stack.size()<=1)return error_t::STACK_EMPTY;

    attribute_block=false;

    auto& ctx = stack.top();
    node_t* parent = (node_t*)(buffer.data()+ctx.first);
    parent->_size=buffer.size()-ctx.first;

    stack.pop();

    return error_t::OK;
}

Builder::error_t Builder::attr(std::string_view name, std::string_view value, std::string_view ns){
    if(open==false)return error_t::TREE_CLOSED;
    if(attribute_block==false)return error_t::TREE_ATTR_CLOSED;

    buffer.resize(buffer.size()+sizeof(attr_t));

    auto& old_ctx = stack.top();

    node_t* parent = (node_t*)(buffer.data()+old_ctx.first);

    //Emplace node
    new ((attr_t*) & (uint8_t&) *( buffer.end()-sizeof(attr_t) )) attr_t(label_offset,ns,name,value);

    parent->attrs_count++;

    return error_t::OK;
}

std::expected<WrpTree,Builder::error_t> Builder::close(){
    if(open==false)return std::unexpected(error_t::TREE_CLOSED);
    open=false;
    if(stack.size()!=1)return std::unexpected(error_t::MISFORMED);
    stack.pop();
    return WrpTree(Tree(std::move(buffer),{}));
}


sv BuilderCompressed::symbol(std::string_view s){
    if(s.length()==0)return {0,0};

    auto it = idx_symbols.find(sv(label_offset,s));

    if(it==idx_symbols.end()){
        symbols.insert(symbols.end(),s.begin(),s.end());
        label_offset=symbols.data();    //Keeep this updated so that when sv are used they are always relative.

        sv ret(symbols.size()-s.length(),s.length());
        auto it = idx_symbols.insert(ret); 
        if(it.second==false)throw std::runtime_error("Unable to insert symbol in symbol table");
        return *it.first;
    }
    else{
        return *it;
    }
}

std::expected<WrpTree,BuilderCompressed::error_t> BuilderCompressed::close(){
    idx_symbols.clear();
    return Builder::close(std::move(symbols));
}

}