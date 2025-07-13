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

#include <endian.h>
#include <expected>


#include <span>
#include <string_view>

#include <bit>
#include <type_traits>

#if __has_include(<vs-xml/config.hpp>)
#include <vs-xml/config.hpp>
#endif

#ifndef VS_XML_NS
#define VS_XML_NS xml
#endif

#ifndef VS_XML_NO_EXCEPT
#define VS_XML_NO_EXCEPT false
#endif

#ifndef VS_XML_LAYOUT
#define VS_XML_LAYOUT 0
#endif


#ifndef VS_XML_NO_ASSERT
#include <vs-xml/fwd/assert.hpp>
#endif 

/**
 * @brief The main namespace for the application. By default `xml` and controlled via the macro VS_XML_NS.
 * 
 */
namespace VS_XML_NS{

constexpr static inline int format_major = 0; ///Current binary format major revision. Major revisions are breaking.
constexpr static inline int format_minor = 0; ///Current binary format minor revision. Minor revisions are not breaking, but older does not support recent.

#if VS_XML_LAYOUT == 0
typedef std::ptrdiff_t delta_ptr_t ;
typedef size_t xml_size_t;
typedef size_t xml_count_t;
typedef size_t xml_enum_size_t;

#elif VS_XML_LAYOUT == 1
typedef int32_t delta_ptr_t ;
typedef uint32_t xml_size_t;
typedef uint16_t xml_count_t;
typedef uint8_t xml_enum_size_t;

#endif

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
    inline sv(const void* offset, std::string_view v){base=((uint8_t*)v.data()-(uint8_t*)offset);length=v.length();}

    inline sv() = default;
    inline sv(const sv& s) = default;
};

/**
 * @brief Configuraton structure for builders, trees and documents.
 * 
 */
struct __attribute__ ((packed)) builder_config_t{
    enum symbols_t: uint8_t{
        EXTERN_ABS,                         //It was `false` in the previous flag. Cannot be saved as binary.
        EXTERN_REL,                         //Can be saved as binary, but it requires the tree to bind a symbols table later.
        OWNED,                              //No compression of symbols, but they are owned.
        COMPRESS_LABELS,                    //Only compressing symbols which are used for tag and prop labels.
        COMPRESS_ALL,                       //It was `true` in the previous flag. Compress all symbols.
        COMPRESS_CUSTOM,                    //TODO: Not implement yet. Reserved for later usage, where an external implementation is provided to compress symbols. This way vsxml does not have to care about shipping expensive compression algorithms.
    } symbols : 3 = EXTERN_ABS;
    bool raw_strings :1  = false;           //If true, the string views being passed will not be de-escaped. XML serialization of the derived tree will have to escape them if they have been de-escaped.
    bool allow_comments :1 = true;          //If true, comments are allowed. Else skip.
    bool allow_procs :1  = true;            //If true, processing nodes are allowed. Else skip.
    bool inline_index :1  = false;          //If true, each node has an inline index of all children appended at the very end, to allow randon-access at O(1) to the n-th child. Not practical for HUGE trees.
};

/**
 * @brief Data structure used to represent a tree (or document), for saving/loading
 * 
 */
 
struct __attribute__ ((packed)) binary_header_t{
    const uint8_t magic[4] = {'$','X','M','L'};
    uint8_t format_major = VS_XML_NS::format_major;
    uint8_t format_minor = VS_XML_NS::format_minor;
    //TODO: add additional format version for the the software release which generated it. To avoid problems like https://github.com/lazy-eggplant/vs.xml/commit/707b3f0a3ba0e3ec7fab0c508b9cb37c7308b26c
    builder_config_t configs;
    enum struct endianess_t {LITTLE,BIG} endianess : 1 = 
        std::endian::native==std::endian::little?
            binary_header_t::endianess_t::LITTLE:
            binary_header_t::endianess_t::BIG;
    uint8_t res0 : 7;

    uint32_t size__delta_ptr : 6        = sizeof(delta_ptr_t);
    uint32_t size__xml_size  : 6        = sizeof(xml_size_t);
    uint32_t size__xml_count : 6        = sizeof(xml_count_t);
    uint32_t size__xml_enum_size : 6    = sizeof(xml_enum_size_t);
    uint32_t res1: 32-24;

    uint16_t docs_count = 1;
    uint8_t res[2];

    uint64_t length_of_symbols; //(excluding padding)

    struct __attribute__ ((packed)) section_t{
        //Cannot use sv because not POD.
        struct __attribute__ ((packed)){
            delta_ptr_t  base;      //Aligned to size() (symbol start)
            xml_count_t  length;    //Aligned to base
        } name;
        delta_ptr_t     base;      //Aligned to start_data
        xml_count_t     length;    //Relative to base
    } sections [];

    constexpr inline section_t region(size_t n) const{
        xml_assert(n<docs_count, "Exceeded maximum documents recorded in binary");
        return sections[n];
    }

    constexpr inline size_t size() const {
        return sizeof(binary_header_t)+sizeof(binary_header_t::section_t)*docs_count;
    }

    constexpr inline size_t start_data() const {
        auto padding = (size()+length_of_symbols%16==0)?0:(16-(size()+length_of_symbols)%16);
        return size()+length_of_symbols+padding;
    }
};
static_assert(offsetof(binary_header_t,sections)%sizeof(uint64_t)==0,"Misaligned section_t in header");


struct element_t;
struct attr_t;
struct text_t;
struct comment_t;
struct cdata_t;
struct proc_t;
struct marker_t;
struct root_t;
struct unknown_t;

struct node_iterator;
struct attr_iterator;
struct text_iterator;
struct visitor_iterator;

namespace wrp{
    template <typename T>
    struct base_t;
    struct sv;

    struct node_iterator;
    struct attr_iterator;
    struct text_iterator;
    struct visitor_iterator;
}

namespace details{
    struct BuilderBase;
}
template<builder_config_t cfg>
struct Builder;

struct Tree;
struct Document;
struct Archive;

enum struct feature_t{
    OK,
    NONE,
    ERROR,
    NOT_SUPPORTED,
    NOT_IMPLEMENTED,
};

/**
 * @brief All types of nodes represented. Some are special values not part of XML
 * 
 */
enum struct type_t : xml_enum_size_t{
    UNKNOWN,    ///Used for dynamic dispatching
    ELEMENT,
    ATTR,
    TEXT,
    CDATA,
    COMMENT,
    PROC,
    MARKER,     ///Special type used to represent injection point or annotations.
};

//TODO: At the moment not really used. Either remove this or fix it to do something. Anything.

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
};

//TODO: specialization of Builder_t or just remove it?
template <typename T>
concept ProperBuilder =  true;  

template<ProperBuilder Builder_t>
class Parser;

template<typename T>
struct StorageFor;

template <typename T, typename... Args>
concept IStorable = requires(const StorageFor<T>& v, Args... args){
   {StorageFor<T>::bind(v,args...)} -> std::same_as<T>;
};

template<typename T>
struct Stored : private StorageFor<T>, T{
    inline Stored(auto&&... args):
        StorageFor<T>(std::forward<decltype(args)>(args)...),
        T(StorageFor<T>::bind(*this,std::forward<decltype(args)>(args)...))
    {
        //static_assert(IStorable<T,decltype(args)...>);
    }
};

///Namespace hosting wrapped versions for most structures with internal data ownership (no good for embedded).
namespace stored{}

}