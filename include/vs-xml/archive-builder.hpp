#pragma once

/**
 * @file archive-builder.hpp
 * @author karurochari
 * @brief Archive builder wrapper implementation for tree builder
 * @date 2025-06-21
 * 
 * @copyright Copyright (c) 2025
 * 
 */


#include "vs-xml/tree-builder.hpp"
#include <vs-xml/commons.hpp>
#include <vs-xml/archive.hpp>
#include <vs-xml/document-builder.hpp>

namespace VS_XML_NS{

template<builder_config_t cfg = {}>
struct ArchiveBuilder{
    private:
        DocBuilder<cfg> builder;
        std::vector<binary_header_t::section_t> fragments;

    public:

    struct reserve_t : TreeBuilder<cfg>::reserve_t{
        size_t fragments;
    };

    inline void reserve(reserve_t sizes){
        fragments.reserve(sizes.fragments);
        return builder.reserve(sizes);
    }
    
    /*
    template<typename... Args>
    [[nodiscard]] constexpr inline details::BuilderBase::error_t document(std::string_view docname, void(*items)(DocBuilder<cfg>&, Args&&... args), Args... args){
        items(builder,std::forward<decltype(args)>(args)...); //TODO: maybe return values should be handled.
        //Register the current frame in a vector for later usage.
        if(auto t = builder.close_frame(docname); t.has_value())fragments.emplace_back(*t);
        else{return t.error();}
        return details::BuilderBase::error_t::OK;
    }
    */
    
    [[nodiscard]] constexpr inline details::BuilderBase::error_t document(std::string_view docname, const std::function<void(DocBuilder<cfg>&)>& items){
        items(builder); //TODO: maybe return values should be handled.
        //Register the current frame in a vector for later usage.
        if(auto t = builder.close_frame(docname); t.has_value())fragments.emplace_back(*t);
        else{return t.error();}
        return details::BuilderBase::error_t::OK;
    }

    [[nodiscard]] inline std::expected<stored::Archive,error_t> close(){
        auto [buffer,symbols] = *builder.extract();    
        return stored::Archive(cfg,std::move(fragments),std::move(buffer),std::move(symbols));
    }
};

}