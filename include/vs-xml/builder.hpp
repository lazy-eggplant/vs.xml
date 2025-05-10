#pragma once

/**
 * @file builder.hpp
 * @author karurochari
 * @brief Tree builder classes
 * @date 2025-05-01
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <cstddef>
#include <cassert>
#include <cstdint>

#include <expected>
#include <functional>

#include <unordered_set>
#include <vector>
#include <stack>
#include <string_view>

#include "commons.hpp"
#include "vs-xml/impl.hpp"
#include "wrp-tree.hpp"

namespace VS_XML_NS{

//TODO: Implement collapse data. Only use text, and if a new text or cdata is defined, append to the prev one.

/**
 * @brief Helper class to build an XML tree via commands.
 * 
 */
struct Builder{
    struct config_t{
        bool allow_comments ;
        bool collapse_text ;
    };

    enum struct error_t{
        OK,
        TREE_CLOSED,
        TREE_ATTR_CLOSED,
        STACK_EMPTY,
        MISFORMED,
    };

    private:
    std::vector<uint8_t> buffer;
    bool open = true;               //True if the tree is still open to append things.
    bool attribute_block = false;   //True after a begin to add attributes. It is automatically closed when any other command is triggered.
    
    std::stack<std::pair<ptrdiff_t,ptrdiff_t>> stack;
    config_t cfg;

    template<typename T>
    error_t leaf(std::string_view value);

    protected:
    void* label_offset = nullptr;

    std::expected<WrpTree,error_t> close(std::vector<uint8_t>&& symbols);
    
    public:

    Builder(config_t cfg={true,false});
    std::expected<WrpTree,error_t> close();

    error_t begin(std::string_view name, std::string_view ns="");
    error_t end();

    error_t attr(std::string_view name, std::string_view value, std::string_view ns="");
 
    inline error_t text(std::string_view value){return leaf<text_t>(value);}
    inline error_t comment(std::string_view value){return leaf<comment_t>(value);}
    inline error_t cdata(std::string_view value){return leaf<cdata_t>(value);}
    inline error_t proc(std::string_view value){return leaf<proc_t>(value);}
    inline error_t marker(std::string_view value){return leaf<marker_t>(value);}

    //TODO: injection can be a simple memcpy if the symbols space is shared, or require full tree refactoring.
    error_t inject(node_iterator start, node_iterator end);
    error_t inject(const Tree& tree);
    error_t inject(const WrpTree& tree);
};

/**
 * @brief Wrapper of `Builder` to compress labels before usage, reducing the final memory footprint.
 * 
 */
struct BuilderCompressed : protected Builder{

    private:
    std::unordered_set<sv, std::function<uint64_t(sv)>,std::function<bool(sv, sv)>> idx_symbols;
    std::vector<uint8_t> symbols;

    /**
     * @brief 
     * NOTICE! The generated string_views have a very narrow lifetime. As symbols changes they are done.
     * Do not let them survive beyon the next change to symbols!
     * @param s 
     * @return std::string_view 
     */
    inline std::string_view rsv(sv s){
        return std::string_view(s.base+(char*)label_offset,s.base+(char*)label_offset+s.length);
    }
  
    sv symbol(std::string_view s);

    public:


    inline BuilderCompressed(config_t cfg = {true,false}):
        Builder(cfg),idx_symbols(64,
        [this](sv a){return std::hash<std::string_view>{}(rsv(a));},
        [this](sv a, sv b){return rsv(a)==rsv(b);}),symbols(){
            label_offset=symbols.data(); 
    }

    inline error_t begin(std::string_view name, std::string_view ns=""){
        auto a =symbol(name), b = symbol(ns);
        return Builder::begin(rsv(a),rsv(b));
    }
    inline error_t end(){
        return Builder::end();
    }
    inline error_t attr(std::string_view name, std::string_view value, std::string_view ns=""){
        auto a =symbol(name), b = symbol(value), c = symbol(ns);
        return Builder::attr(rsv(a),rsv(b),rsv(c));
    }
 
    inline error_t text(std::string_view value){
        return Builder::text(rsv( symbol(value)));
    }
    inline error_t comment(std::string_view value){
        return Builder::comment(rsv( symbol(value)));
    }
    inline error_t cdata(std::string_view value){
        return Builder::cdata(rsv( symbol(value)));
    }
    inline error_t proc(std::string_view value){
        return Builder::proc(rsv( symbol(value)));
    }
    inline error_t marker(std::string_view value){
        return Builder::marker(rsv( symbol(value)));
    }

    std::expected<WrpTree,error_t> close();
};


}