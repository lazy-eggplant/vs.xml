#pragma once

/**
 * @file tree-builder.hpp
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
#include <utility>

#include <vs-xml/fwd/unordered_set.hpp>
#include <vector>
#include <string_view>

#include <vs-xml/commons.hpp>
#include <vs-xml/tree.hpp>
#include <vs-xml/node.hpp>
#include <vs-xml/wrp-node.hpp>

namespace VS_XML_NS{


namespace details{
    template <builder_config_t::symbols_t COMPRESSION>
    struct Symbols{
    };

    template <>
    struct Symbols<builder_config_t::symbols_t::EXTERN_ABS>{
        std::span<const uint8_t> symbols = {};

        using sv_t = std::string_view;

        inline std::string_view rsv(std::string_view s){return s;}
        inline std::string_view label(std::string_view s){return s;}
        inline std::string_view symbol(std::string_view s){return s;}
    };

    template <>
    struct Symbols<builder_config_t::symbols_t::EXTERN_REL>{
        std::string_view symbols;

        using sv_t = sv;

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

        using sv_t = sv;

        sv label(std::string_view s);
        inline sv symbol(std::string_view s){return label(s);}
        inline std::string_view rsv(sv s){return std::string_view(s.base+(char*)symbols.data(),s.base+(char*)symbols.data()+s.length);}

        inline Symbols(){}
    };

    template <>
    struct Symbols<builder_config_t::symbols_t::COMPRESS_ALL> : Symbols<builder_config_t::symbols_t::OWNED>{
        VS_XML_NS::unordered_set<sv, std::function<uint64_t(sv)>,std::function<bool(sv, sv)>> idx;

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
            FRAME_ERROR,
        };
    
        protected:
            std::vector<uint8_t> buffer;

            bool open = true;               //True if the tree is still open to append things.
            bool attribute_block = false;   //True after a begin to add attributes. It is automatically closed when any other command is triggered.
            std::vector<std::pair<ptrdiff_t,ptrdiff_t>> stack;

            const void* symoffset = nullptr;

            delta_ptr_t last_offset = 0;

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

template<builder_config_t cfg = builder_config_t{}>
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
        TreeBuilder(std::string_view src):symbols(src){
            static_assert(cfg.symbols==builder_config_t::EXTERN_REL, "Only EXTERN_REL builders can pass the source symbol table");
            symoffset = symbols.symbols.data();
        }

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
         * @brief Inject a subtree coming from another tree into this builder.
         * @details Every label and value is fed back through this builder's own
         *          symbol table, so source and destination trees may use unrelated
         *          symbol spaces. Source and destination are expected to agree on
         *          the `raw_strings` encoding of values.
         *
         * @param tree the tree to read from
         * @param base the node to start from (defaults to the tree root)
         * @param include_root if true, emit `base` itself, otherwise only its children
         */
        error_t inject(const TreeRaw& tree, const unknown_t* base = nullptr, bool include_root = false){
            const unknown_t* start = base ? base : &tree.root();
            if(include_root && start != &tree.root())return inject_node(tree, start);

            auto range = start->children_range();
            if(!range.has_value())return error_t::MISFORMED;
            for(const unknown_t* c = range->first; c != range->second; c = c->next()){
                if(auto e = inject_node(tree, c); e != error_t::OK)return e;
            }
            return error_t::OK;
        }

    private:
        error_t inject_node(const TreeRaw& tree, const unknown_t* node){
            switch(node->type()){
                case type_t::ELEMENT: {
                    auto name = node->name();
                    if(!name.has_value())return error_t::MISFORMED;
                    auto ns = node->ns();
                    if(auto e = this->begin(
                        tree.rsv(name.value()),
                        ns.has_value() ? tree.rsv(ns.value()) : std::string_view{}
                    ); e != error_t::OK)return e;

                    auto arange = node->attrs_range();
                    if(arange.has_value()){
                        for(const VS_XML_NS::attr_t* a = arange->first; a != arange->second; ++a){
                            auto an = a->name();
                            auto av = a->value();
                            auto ans = a->ns();
                            if(!an.has_value() || !av.has_value())continue;
                            if(auto e = this->attr(
                                tree.rsv(an.value()),
                                tree.rsv(av.value()),
                                ans.has_value() ? tree.rsv(ans.value()) : std::string_view{}
                            ); e != error_t::OK)return e;
                        }
                    }

                    auto crange = node->children_range();
                    if(crange.has_value()){
                        for(const unknown_t* c = crange->first; c != crange->second; c = c->next()){
                            if(auto e = inject_node(tree, c); e != error_t::OK)return e;
                        }
                    }
                    return this->end();
                }
                case type_t::TEXT:    return this->text(tree.rsv(node->value().value_or(VS_XML_NS::sv{0,0})));
                case type_t::CDATA:   return this->cdata(tree.rsv(node->value().value_or(VS_XML_NS::sv{0,0})));
                case type_t::COMMENT: return this->comment(tree.rsv(node->value().value_or(VS_XML_NS::sv{0,0})));
                case type_t::PROC:    return this->proc(tree.rsv(node->value().value_or(VS_XML_NS::sv{0,0})));
                case type_t::MARKER:  return this->marker(tree.rsv(node->value().value_or(VS_XML_NS::sv{0,0})));
                default: return error_t::OK;
            }
        }

    public:
        /**
         * @brief Final operaration when the building process is finished.
         * 
         * @return std::expected<Tree,error_t> a wrapped tree if successful, or an error
         */
        [[nodiscard]] std::expected<stored::Tree,error_t> close(){
            if (auto ret = details::BuilderBase::close(); ret != details::BuilderBase::error_t::OK)return std::unexpected(ret);
            if constexpr (
                cfg.symbols==builder_config_t::symbols_t::COMPRESS_ALL ||
                cfg.symbols==builder_config_t::symbols_t::COMPRESS_LABELS ||
                cfg.symbols==builder_config_t::symbols_t::OWNED 
            )return stored::Tree(configs,std::exchange(buffer,{}),std::exchange(symbols.symbols,{}));
            else return stored::Tree(configs,std::exchange(buffer,{}),symbols.symbols.data());
        }

        /**
         * @brief Special version of the closing function, only returning the generated tree but not symbols.
         * @details The current state for symbols is preserved, and the new tree will inherit them.
         * @return std::expected<std::vector<uint8_t>,error_t> 
         */
        [[nodiscard]] std::expected<binary_header_t::section_t,error_t> close_frame(std::string_view name=""){
            //Record a symbol for the frame name, so that the name string_view can be returned.
            auto sv_name = label(name);
            if (auto ret = details::BuilderBase::close(); ret != details::BuilderBase::error_t::OK)return std::unexpected(ret);
            open=true;
            attribute_block=false;
            stack.push_back({0,-1});
            delta_ptr_t cpy_offset = last_offset;
            last_offset=buffer.size();
            return binary_header_t::section_t{{sv_name.base,sv_name.length},cpy_offset,buffer.size()-cpy_offset};
        }

        /**
         * @brief It allows to extract symbols if the builder has been closed.
         * @details Not to be used with `close` only with `close_frame`
         * @return std::optional<details::Symbols<configs.symbols>> 
         */     
        [[nodiscard]] std::optional<std::pair<std::vector<uint8_t>,std::vector<uint8_t>>> extract(){
            details::BuilderBase::close();
            return std::pair{std::move(buffer),std::move(symbols.symbols)};
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

        constexpr inline void x(std::string_view ns, std::string_view name, const std::initializer_list<attr_t>& attrs, void(*items)(TreeBuilder&)){
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


        constexpr void x(std::string_view name, const std::initializer_list<attr_t>& attrs, void(*items)(TreeBuilder&)){return x("",name,attrs,items);}
        constexpr void x(std::string_view name, const std::initializer_list<attr_t>& attrs={}, const std::function<void()>& items=[](){}){return x("",name,attrs,items);}

        struct reserve_t{
            size_t buffer;
            size_t symbols;
            size_t symbols_index;
        };

        /**
         * @brief Reserves space for the buffer to avoid many of the initial small allocations. Ideally run just after initialization.
         */
        inline void reserve(reserve_t sizes){
            this->buffer.reserve(sizes.buffer);
            if constexpr(configs.symbols==builder_config_t::OWNED || configs.symbols==builder_config_t::COMPRESS_ALL || configs.symbols==builder_config_t::COMPRESS_LABELS){
                symbols.symbols.reserve(sizes.symbols);
            }
            if constexpr(configs.symbols==builder_config_t::COMPRESS_ALL || configs.symbols==builder_config_t::COMPRESS_LABELS){
                symbols.symbols.reserve(sizes.symbols);
                symbols.idx.reserve(sizes.symbols_index);
            }
        }
};

}