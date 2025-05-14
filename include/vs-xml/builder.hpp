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
#include "impl.hpp"
#include "wrp-impl.hpp"

namespace VS_XML_NS{


namespace details{
    struct BuilderBase{
        enum struct error_t{
            SKIP = -1,
            OK = 0,
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
    
            template<typename T>
            error_t leaf(std::string_view value);
    
        protected:
            std::string_view symbols;
            inline std::vector<uint8_t>&& get_buffer() {return std::move(buffer);}
        

            /**
            * @brief 
            * NOTICE! The generated string_views have a very narrow lifetime. As symbols changes they are done.
            * Do not let them survive beyon the next change to symbols!
            * @param s 
            * @return std::string_view 
            */
            inline std::string_view rsv(sv s){
                return std::string_view(s.base+(char*)symbols.data(),s.base+(char*)symbols.data()+s.length);
            }

        public:
    
            BuilderBase();
            error_t close();
    
            error_t begin(std::string_view name, std::string_view ns="");
            error_t end();
    
            error_t attr(std::string_view name, std::string_view value, std::string_view ns="");
    
            inline error_t text(std::string_view value){return leaf<text_t>(value);}
            inline error_t comment(std::string_view value){return leaf<comment_t>(value);}
            inline error_t cdata(std::string_view value){return leaf<cdata_t>(value);}
            inline error_t proc(std::string_view value){return leaf<proc_t>(value);}
            inline error_t marker(std::string_view value){return leaf<marker_t>(value);}
    
            //TODO: injection can be a simple memcpy if the symbols space is shared, or require full tree refactoring.
            error_t inject(const TreeRaw& tree, const unknown_t* base = nullptr, bool include_root = false);

//            error_t inject(const Tree& tree);
    };
    
/**
 * @brief Helper class to build an XML tree via commands.
 * 
 */

template <builder_config_t::symbols_t COMPRESSION>
struct BuilderImpl: protected BuilderBase{
    protected:
        using BuilderBase::close;
        using BuilderBase::get_buffer;

    public:
        using BuilderBase::inject;
};

template <>
struct BuilderImpl<builder_config_t::symbols_t::OWNED>: BuilderBase{
    protected:    
        std::vector<uint8_t> symbols_i;
        sv symbol(std::string_view s);

    public:   
        inline BuilderImpl():BuilderBase(){
            symbols= std::string_view((char*)symbols.data(),(char*)symbols.data()+symbols.size()); 
        }
};

template <>
struct BuilderImpl<builder_config_t::symbols_t::COMPRESS_ALL>: BuilderImpl<builder_config_t::symbols_t::OWNED>{
    protected:
        std::unordered_set<sv, std::function<uint64_t(sv)>,std::function<bool(sv, sv)>> idx_symbols;
        sv symbol(std::string_view s);

    public:
        inline BuilderImpl():idx_symbols(64,
        [this](sv a){return std::hash<std::string_view>{}(rsv(a));},
        [this](sv a, sv b){return rsv(a)==rsv(b);}){
            symbols= std::string_view((char*)this->symbols_i.data(),(char*)symbols_i.data()+symbols_i.size()); 
        }
};


template <>
struct BuilderImpl<builder_config_t::symbols_t::COMPRESS_LABELS>: BuilderImpl<builder_config_t::symbols_t::COMPRESS_ALL>{
    protected:
        sv symbol(std::string_view s);
};


template <>
struct BuilderImpl<builder_config_t::symbols_t::EXTERN_ABS> : BuilderBase{
    protected:
        inline std::string_view rsv(std::string_view s){return s;}
        inline std::string_view symbol(std::string_view s){return s;}
};

template <>
struct BuilderImpl<builder_config_t::symbols_t::EXTERN_REL> : BuilderImpl<builder_config_t::symbols_t::EXTERN_ABS> {
    protected:
        //TODO: Add checks?
        inline std::string_view rsv(sv s){return std::string_view(s.base+(char*)symbols.data(),s.base+(char*)symbols.data()+s.length);}
        inline sv symbol(std::string_view s){return sv(symbols.data(),s);}

        inline BuilderImpl(std::string_view src){
            this->symbols = src;
        }

        BuilderImpl() = delete;
};

}

template<builder_config_t cfg = {}>
struct TreeBuilder : protected details::BuilderImpl<cfg.symbols>{
    protected:
        using details::BuilderImpl<cfg.symbols>::symbol;

        //TODO: not sure if this is needed any longer
        /*inline std::expected<Tree,error_t> close(std::vector<uint8_t>&& symbols){
            details::BuilderImpl<cfg.compress_symbols>::close();
            return Tree(Tree(config,std::move(this->buffer),std::move(symbols)));
        }*/

    public:
        constexpr static inline builder_config_t configs = cfg;
        constexpr static inline bool is_document = false;

        using details::BuilderImpl<cfg.symbols>::rsv;
        using error_t = details::BuilderBase::error_t;

        //inline Builder():details::BuilderImpl<cfg.compress_symbols>(){}

        inline error_t begin(std::string_view name, std::string_view ns=""){
            auto a =symbol(name), b = symbol(ns);
            return details::BuilderBase::begin(rsv(a),rsv(b));
        }
        inline error_t end(){
            return details::BuilderBase::end();
        }
        inline error_t attr(std::string_view name, std::string_view value, std::string_view ns=""){
            auto a =symbol(name), b = symbol(value), c = symbol(ns);
            return details::BuilderBase::attr(rsv(a),rsv(b),rsv(c));
        }

        inline error_t text(std::string_view value){
            return details::BuilderBase::text(rsv( symbol(value)));
        }
        inline error_t comment(std::string_view value){
            if constexpr(!cfg.allow_comments)return error_t::SKIP;
            else return details::BuilderBase::comment(rsv( symbol(value)));
        }
        inline error_t cdata(std::string_view value){
            return details::BuilderBase::cdata(rsv( symbol(value)));
        }
        inline error_t proc(std::string_view value){
            if constexpr(!cfg.allow_procs)return error_t::SKIP;
            else return details::BuilderBase::proc(rsv( symbol(value)));
        }
        inline error_t marker(std::string_view value){
            return details::BuilderBase::marker(rsv( symbol(value)));
        }

        inline std::expected<Tree,error_t> close(){
            details::BuilderImpl<cfg.symbols>::close();
            if constexpr (
                cfg.symbols==builder_config_t::symbols_t::COMPRESS_ALL ||
                cfg.symbols==builder_config_t::symbols_t::COMPRESS_LABELS ||
                cfg.symbols==builder_config_t::symbols_t::OWNED 
            )return Tree(TreeRaw(configs,std::move(this->get_buffer()),std::move(this->symbols_i)));
            else return Tree(TreeRaw(configs,std::move(this->get_buffer()),this->symbols.data()));
        }
        
        using details::BuilderImpl<cfg.symbols>::close;
        using details::BuilderImpl<cfg.symbols>::inject;

};

}