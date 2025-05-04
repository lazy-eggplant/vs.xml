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


namespace xml{

typedef std::ptrdiff_t delta_ptr_t ;
typedef size_t xml_size_t;
typedef size_t xml_count_t;
typedef size_t xml_enum_size_t;

inline void xml_assert(bool b){return assert(b);}

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

struct node_t;
struct attr_t;
struct text_t;
struct comment_t;
struct cdata_t;
struct proc_t;
struct inject_t;
struct unknown_t;

struct Parser;
struct Builder;
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
    NODE,
    ATTR,
    TEXT,
    COMMENT,
    PROC,
    CDATA,
    INJECT,
};

template<typename T>
concept thing_i = requires(T self){
    {self.type()} -> std::same_as<type_t>;
    {self.ns()} -> std::same_as<std::expected<sv,feature_t>>;
    {self.name()} -> std::same_as<std::expected<sv,feature_t>>;
    {self.value()} -> std::same_as<std::expected<sv,feature_t>>;

    {self.children()} -> std::same_as<std::expected<std::pair<const unknown_t*, const unknown_t*>,feature_t>>;
    {self.attrs()} -> std::same_as<std::expected<std::pair<const attr_t*, const attr_t*>,feature_t>>;

    {self.parent()} -> std::same_as<const node_t*>;
    {self.prev()} -> std::same_as<const unknown_t*>;
    {self.next()} -> std::same_as<const unknown_t*>;

    {self.has_parent()} -> std::same_as<bool>;
    {self.has_prev()} -> std::same_as<bool>;
    {self.has_next()} -> std::same_as<bool>;

    {self.path()} -> std::same_as<std::string>;
};


}