#pragma once

/**
 * @file common.hpp
 * @author karurochari
 * @brief Common utilities, concepts etc.
 * @date 2025-05-01
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <cstddef>
#include <cassert>
#include <cstdint>

#include <concepts>

#include <expected>


#include <string>
#include <string_view>

#ifndef VS_XML_NO_ASSERT
#include "assert.hpp"
#endif 

#ifndef VS_XML_NS
#define VS_XML_NS xml
#endif

namespace VS_XML_NS{


typedef std::ptrdiff_t delta_ptr_t ;
typedef size_t xml_size_t;
typedef size_t xml_count_t;
typedef size_t xml_enum_size_t;


inline void xml_assert(bool condition, const char* errorMessage="") {
    #ifndef VS_XML_NO_ASSERT
    modern_assert(condition,errorMessage);
    #endif 
}

/**
 * @brief Compact position-free string_view
 * 
 */
struct sv{
    delta_ptr_t  base;
    xml_count_t  length;

    inline sv(std::ptrdiff_t b, size_t len):base(b),length(len){xml_assert((std::ptrdiff_t)base == b);xml_assert((xml_count_t)length == len);}
    inline sv(void* offset, std::string_view v){base=((uint8_t*)v.data()-(uint8_t*)offset);length=v.length();}

    sv()=delete;
};

struct builder_config_t{
    bool allow_comments :1 = true;          //If true, comments are allowed. Else skip.
    bool allow_procs :1  = true;            //If true, processing nodes are allowed. Else skip.
    bool compress_symbols :1  = false;      //If true, a new table of symbols is computed, else string_views are used directly.
    bool raw_strings :1  = false;           //If true, the string views being passed will not be de-escaped. XML serialization of the derived tree will have to escape them if they have been de-escaped.
};

struct element_t;
struct attr_t;
struct text_t;
struct comment_t;
struct cdata_t;
struct proc_t;
struct marker_t;
struct root_t;
struct unknown_t;

namespace wrp{
    template <typename T>
    struct base_t;
}

namespace details{
    struct BuilderBase;
}
template<builder_config_t cfg>
struct Builder;

struct Tree;
struct Tree;


enum struct feature_t{
    OK,
    NONE,
    ERROR,
    NOT_SUPPORTED,
    NOT_IMPLEMENTED,
};

enum struct type_t : xml_enum_size_t{
    UNKNOWN,
    ROOT,
    ELEMENT,
    ATTR,
    TEXT,
    COMMENT,
    PROC,
    CDATA,
    MARKER,
};

template<typename T>
concept thing_i = requires(T self){
    {self.type()} -> std::same_as<type_t>;
    {self.ns()} -> std::same_as<std::expected<sv,feature_t>>;
    {self.name()} -> std::same_as<std::expected<sv,feature_t>>;
    {self.value()} -> std::same_as<std::expected<sv,feature_t>>;

    {self.children_range()} -> std::same_as<std::expected<std::pair<const unknown_t*, const unknown_t*>,feature_t>>;
    {self.attrs_range()} -> std::same_as<std::expected<std::pair<const attr_t*, const attr_t*>,feature_t>>;
    {self.text_range()} -> std::same_as<std::expected<void,feature_t>>;

    {self.parent()} -> std::same_as<const element_t*>;
    {self.prev()} -> std::same_as<const unknown_t*>;
    {self.next()} -> std::same_as<const unknown_t*>;

    {self.has_parent()} -> std::same_as<bool>;
    {self.has_prev()} -> std::same_as<bool>;
    {self.has_next()} -> std::same_as<bool>;

    {self.path()} -> std::same_as<std::string>;
};


template <typename T>
concept ProperBuilder =  true;  //TODO: specialization of Builder_t

template<ProperBuilder Builder_t>
class Parser;



}