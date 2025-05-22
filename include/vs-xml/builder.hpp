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


#include <vs-xml/private/unordered_set.hpp>
#include <vector>
#include <string_view>

#include <vs-xml/commons.hpp>
#include <vs-xml/private/impl.hpp>
#include <vs-xml/private/wrp-impl.hpp>

namespace VS_XML_NS{


namespace details{
    template <builder_config_t::symbols_t COMPRESSION>
    struct Symbols{
    };

    template <>
    struct Symbols<builder_config_t::symbols_t::EXTERN_ABS>{
        std::string_view symbols = {nullptr,std::span<uint8_t>::extent};

        inline std::string_view rsv(std::string_view s){return s;}
        inline std::string_view label(std::string_view s){return s;}
        inline std::string_view symbol(std::string_view s){return s;}
    };

    template <>
    struct Symbols<builder_config_t::symbols_t::EXTERN_REL>{
        std::string_view symbols;

        //TODO: Add checks?
        inline std::string_view rsv(sv s){return std::string_view(s.base+(char*)symbols.data(),s.base+(char*)symbols.data()+s.length);}
        inline sv label(std::string_view s){return sv(symbols.data(),s);}
        inline sv symbol(std::string_view s){return sv(symbols.data(),s);}

        inline Symbols(std::string_view src){
            this->symbols = src;
        }
    };

    template <>
    struct Symbols<builder_config_t::symbols_t::OWNED>{
        std::vector<uint8_t> symbols;

        sv label(std::string_view s);
        inline sv symbol(std::string_view s){return label(s);}
        inline std::string_view rsv(sv s){return std::string_view(s.base+(char*)symbols.data(),s.base+(char*)symbols.data()+s.length);}

        inline Symbols(){}
    };

    template <>
    struct Symbols<builder_config_t::symbols_t::COMPRESS_ALL> : Symbols<builder_config_t::symbols_t::OWNED>{
        xml::unordered_set<sv, std::function<uint64_t(sv)>,std::function<bool(sv, sv)>> idx;

        sv label(std::string_view s);
        inline sv symbol(std::string_view s){return label(s);}

        inline Symbols():idx(64,
        [this](sv a){return std::hash<std::string_view>{}(rsv(a));},
        [this](sv a, sv b){return rsv(a)==rsv(b);}){}
    };

    template <>
    struct Symbols<builder_config_t::symbols_t::COMPRESS_LABELS> : Symbols<builder_config_t::symbols_t::COMPRESS_ALL>{
        sv symbol(std::string_view s);
    };


    struct BuilderBase{
        enum struct error_t{
            SKIP = -1,
            OK = 0,
            TREE_CLOSED,
            TREE_ATTR_CLOSED,
            STACK_EMPTY,
            MISFORMED,
        };
    
        protected:
            std::vector<uint8_t> buffer;

            bool open = true;               //True if the tree is still open to append things.
            bool attribute_block = false;   //True after a begin to add attributes. It is automatically closed when any other command is triggered.
            std::vector<std::pair<ptrdiff_t,ptrdiff_t>> stack;

            const void* symoffset = nullptr;

            template<typename T>
            error_t leaf(std::string_view value);

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
    };
    
}

template<builder_config_t cfg = {}>
struct TreeBuilder : details::BuilderBase{
    using error_t = details::BuilderBase::error_t;

    protected:
        details::Symbols<cfg.symbols> symbols;

        //Forwarding functions from symbols and updating symoffset if needed.
        inline auto label(auto a){
            auto tmp = symbols.label(a);
            if constexpr(cfg.symbols==builder_config_t::OWNED || cfg.symbols==builder_config_t::COMPRESS_ALL || cfg.symbols==builder_config_t::COMPRESS_LABELS) symoffset = symbols.symbols.data();
            return tmp;
        }
        inline auto symbol(auto a){
            auto tmp = symbols.symbol(a);
            if constexpr(cfg.symbols==builder_config_t::OWNED || cfg.symbols==builder_config_t::COMPRESS_ALL || cfg.symbols==builder_config_t::COMPRESS_LABELS) symoffset = symbols.symbols.data();
            return tmp;
        }
        inline auto rsv(auto a){return symbols.rsv(a);}
        

    public:
        //TODO: rework to accept a configuration object with the reservations of all vectors
        TreeBuilder(std::string_view src):symbols(src){
            static_assert(cfg.symbols==builder_config_t::EXTERN_REL, "Only EXTERN_REL builders can pass the source symbol table");
            symoffset = symbols.symbols.data();
        }

        //TODO: rework to accept a configuration object with the reservations of all vectors
        TreeBuilder(){
            static_assert(cfg.symbols!=builder_config_t::EXTERN_REL, "EXTERN_REL cannot build without a source symbol table");
            symoffset = symbols.symbols.data();
        }

        constexpr static inline builder_config_t configs = cfg;
        constexpr static inline bool is_document = false;

        inline error_t begin(std::string_view name, std::string_view ns=""){
            auto a =label(name), b = label(ns);
            return details::BuilderBase::begin(rsv(a),rsv(b));
        }
        inline error_t end(){
            return details::BuilderBase::end();
        }
        inline error_t attr(std::string_view name, std::string_view value, std::string_view ns=""){
            auto a =label(name), b = symbol(value), c = label(ns);
            return details::BuilderBase::attr(rsv(a),rsv(b),rsv(c));
        }
        inline error_t text(std::string_view value){
            return details::BuilderBase::text(rsv( symbol(value)));
        }
        inline error_t comment(std::string_view value){
            if constexpr(!cfg.allow_comments)return error_t::SKIP;
            return details::BuilderBase::comment(rsv( symbol(value)));
        }
        inline error_t cdata(std::string_view value){
            return details::BuilderBase::cdata(rsv( symbol(value)));
        }
        inline error_t proc(std::string_view value){
            if constexpr(!cfg.allow_procs)return error_t::SKIP;
            return details::BuilderBase::proc(rsv( symbol(value)));
        }
        inline error_t marker(std::string_view value){
            return details::BuilderBase::marker(rsv( symbol(value)));
        }

        /**
         * @brief Final operaration when the building process is finished.
         * 
         * @return std::expected<Tree,error_t> a wrapped tree if successful, or an error
         */
        [[nodiscard]] inline std::expected<Tree,error_t> close(){
            auto ret = details::BuilderBase::close();
            if(ret!=details::BuilderBase::error_t::OK)return std::unexpected(ret);
            if constexpr (
                cfg.symbols==builder_config_t::symbols_t::COMPRESS_ALL ||
                cfg.symbols==builder_config_t::symbols_t::COMPRESS_LABELS ||
                cfg.symbols==builder_config_t::symbols_t::OWNED 
            )return Tree(TreeRaw(configs,std::move(buffer),std::move(symbols.symbols)));
            else return Tree(TreeRaw(configs,std::move(buffer),symbols.symbols.data()));
        }

        /**
         * @brief Special version of the closing function, only returning the generated tree but not symbols.
         * @details The current state for symbols is preserved, and the new tree will inherit them.
         * @return std::expected<std::vector<uint8_t>,error_t> 
         */
        [[nodiscard]] inline std::expected<std::vector<uint8_t>,error_t> close_frame(){
            auto ret = details::BuilderBase::close();
            if(ret!=details::BuilderBase::error_t::OK)return std::unexpected(ret);
            open=true;
            attribute_block=false;
            return std::move(buffer);
        }

        /**
         * @brief It allows to extract symbols if the builder has been closed.
         * @details Not to be used with `close` only with `close_frame`
         * @return std::optional<details::Symbols<configs.symbols>> 
         */
        [[nodiscard]] inline std::optional<details::Symbols<configs.symbols>> extract_symbols(){
            if(open==true)return {};
            else return std::move(symbols);
        }
        
        /**
         * @brief Fork the current builder instance by cloning its current state into a new one.
         * 
         * @return TreeBuilder 
         */
        [[nodiscard]] inline TreeBuilder fork() const{
            return TreeBuilder(*this);
        }

        struct attr_t{
            std::string_view name;
            std::string_view value;
            std::string_view ns;
        };

        constexpr void x(std::string_view ns, std::string_view name, const std::initializer_list<attr_t>& attrs, const std::function<void(TreeBuilder&)>& items){
            begin(name,ns);

            for(auto& a:attrs){
                attr(a.name,a.value,a.ns);
            }

            items(*this);

            end();
        }

        constexpr void x(std::string_view ns, std::string_view name, const std::initializer_list<attr_t>& attrs={}, const std::function<void()>& items=[](){}){
            begin(name,ns);

            for(auto& a:attrs){
                attr(a.name,a.value,a.ns);
            }

            items();

            end();
        }

        constexpr void x(std::string_view name, const std::initializer_list<attr_t>& attrs, const std::function<void(TreeBuilder&)>& items){return x("",name,attrs,items);}
        constexpr void x(std::string_view name, const std::initializer_list<attr_t>& attrs={}, const std::function<void()>& items=[](){}){return x("",name,attrs,items);}


        /**
         * @brief Reserves space for the buffer to avoid many of the initial small allocations. Ideally run just after initialization.
         * 
         * @param bytes 
         */
        inline void reserve(size_t bytes,size_t bytes2=0){
            this->buffer.reserve(bytes);
            if constexpr(configs.symbols==builder_config_t::OWNED || configs.symbols==builder_config_t::COMPRESS_ALL || configs.symbols==builder_config_t::COMPRESS_LABELS){
                symbols.symbols.reserve(bytes2);
            }
        }
};

}