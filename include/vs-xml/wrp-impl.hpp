#pragma once

#include "tree.hpp"
#include "commons.hpp"
#include "impl.hpp"
#include <cstddef>
#include <iterator>
#include <string_view>

namespace VS_XML_NS{

namespace wrp{

struct node_iterator;
struct attr_iterator;

struct sv  {
    //Used to make combinations of string_views and sv work together as intended. 
    // A small price on stack for better usability. These objects are just temp so it is not a concern.
    private:
        const TreeRaw* tree;

        union{
            VS_XML_NS::sv main;
            std::string_view alt = {};  
        }body;

    public:

    inline sv(const TreeRaw& tree, std::string_view v):tree(&tree),body({tree.symbols.data(),v}){}
    inline sv(const TreeRaw& tree, VS_XML_NS::sv v):tree(&tree),body({v}){}
    inline sv(std::string_view v):tree(nullptr){body.alt = v;}
    inline sv(const char v[]):tree(nullptr){body.alt = v;}

    operator std::string_view() const {if(tree!=nullptr)return tree->rsv(body.main);else return body.alt;}

    //TODO: Add comparison operators. Since they can access the tree configs, it is possible to know if serialization is needed or not, picking the optimal comparison algorithm.
};

template <typename T>
struct base_t{
    private:
        const TreeRaw* base;
        const T*       ptr;
    
        base_t(const TreeRaw& base, ptrdiff_t ptr):base(&base),ptr((const T* )((const char*)base.buffer.data()+ptr)){}
        base_t(const TreeRaw& base, const T* ptr):base(&base),ptr(ptr){}
        base_t(base_t p, const T* ptr):base(p.base),ptr(ptr){}
        base_t() = default;

        friend struct VS_XML_NS::Tree;
        friend struct VS_XML_NS::Document;
        template <typename W>
        friend struct base_t;

        friend struct node_iterator;
        friend struct attr_iterator;
    public:
    
    base_t(const base_t& ) = default;

    inline explicit operator const T*() const  {return ptr;}

    inline ptrdiff_t portable() const{return {T::type(),ptr-base->buffer.data()};}

    inline std::expected<sv,feature_t> ns() const{auto tmp = ptr->ns(); if(!tmp.has_value())return std::unexpected{tmp.error()}; else return sv(*base,*tmp);}
    inline std::expected<sv,feature_t> name() const{auto tmp = ptr->name(); if(!tmp.has_value())return std::unexpected{tmp.error()}; else return sv(*base,*tmp);}
    inline std::expected<sv,feature_t> value() const{auto tmp = ptr->value(); if(!tmp.has_value())return std::unexpected{tmp.error()}; else return sv(*base,*tmp);}

    inline std::expected<std::pair<base_t<unknown_t>, base_t<unknown_t>>,feature_t> children_range() const{
        auto tmp = ptr->children_range();
        if(!tmp.has_value())return std::unexpected{tmp.error()};
        else return std::pair{base_t<unknown_t>(*base,tmp->first), base_t<unknown_t>(*base,tmp->second)};
    }
    inline std::expected<std::pair<const attr_t*, const attr_t*>,feature_t> attrs_range() const{return ptr->attrs_range();}

    inline base_t<element_t> parent() const {return {*base,ptr->parent()};}
    inline base_t<unknown_t> prev() const {return {*base,ptr->prev()};}
    inline base_t<unknown_t> next() const {return {*base,ptr->next()};}

    inline bool has_parent() const{return ptr->has_parent();}
    inline bool has_prev() const{return ptr->has_prev();}
    inline bool has_next() const{return ptr->has_next();}

    inline auto attrs() const;
    inline auto attrs(auto filter) const;
    inline auto children() const;
    inline auto children(auto filter) const;

    inline auto type() const {return ptr->type();}
};


template <>
struct base_t<attr_t>{
    private:
        const TreeRaw* base;
        const attr_t*  ptr;
    
        base_t(const TreeRaw& base, ptrdiff_t ptr):base(&base),ptr((const attr_t* )((const char*)base.buffer.data()+ptr)){}
        base_t(const TreeRaw& base, const attr_t* ptr):base(&base),ptr(ptr){}
        base_t(base_t p, const attr_t* ptr):base(p.base),ptr(ptr){}
        base_t() = default;

        friend struct Tree;
        template <typename W>
        friend struct wrp::base_t;

        friend struct wrp::node_iterator;
        friend struct wrp::attr_iterator;
    public:

    base_t(const base_t&) = default;

    inline explicit operator const attr_t*() const  {return ptr;}

    inline ptrdiff_t portable() const{return (const uint8_t*)ptr-base->buffer.data();}

    inline std::expected<sv,feature_t> ns() const{auto tmp = ptr->ns(); if(!tmp.has_value())return std::unexpected{tmp.error()}; else return sv(*base,*tmp);}
    inline std::expected<sv,feature_t> name() const{auto tmp = ptr->name(); if(!tmp.has_value())return std::unexpected{tmp.error()}; else return sv{*base,*tmp};}
    inline std::expected<sv,feature_t> value() const{auto tmp = ptr->value(); if(!tmp.has_value())return std::unexpected{tmp.error()}; else return sv{*base,*tmp};}

    inline auto type() const {return type_t::ATTR;}
};


struct node_iterator{
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = base_t<unknown_t>;
    using pointer           = base_t<unknown_t>;
    using reference         = base_t<unknown_t>;

    inline node_iterator(base_t<unknown_t> ptr) : m_ptr(ptr) {}
    inline node_iterator() = default;
    inline node_iterator(const node_iterator&) = default;

    inline const base_t<unknown_t>& operator*() const { return m_ptr; }
    inline const unknown_t* operator->() { return m_ptr.ptr; }

    inline node_iterator& operator++() { m_ptr.ptr = m_ptr.ptr->next(); return *this; }  
    inline node_iterator& operator--() { m_ptr.ptr = m_ptr.ptr->prev(); return *this; }  

    inline node_iterator operator++(int) { node_iterator tmp = *this; ++(*this); return tmp; }
    inline node_iterator operator--(int) { node_iterator tmp = *this; --(*this); return tmp; }

    inline friend bool operator== (const node_iterator& a, const node_iterator& b) { return (a.m_ptr.base == b.m_ptr.base) && (a.m_ptr.ptr == b.m_ptr.ptr); };
    inline friend bool operator!= (const node_iterator& a, const node_iterator& b) { return (a.m_ptr.base != b.m_ptr.base) || (a.m_ptr.ptr != b.m_ptr.ptr); };  

    private:
    base_t<unknown_t> m_ptr;
};

//TODO: it should be  std::random_access_iterator_tag;
struct attr_iterator{
    using iterator_category = std::bidirectional_iterator_tag; 
    using difference_type   = std::ptrdiff_t;
    using value_type        = base_t<attr_t>;
    using pointer           = base_t<attr_t>;
    using reference         = base_t<attr_t>;

    inline attr_iterator(base_t<attr_t> ptr) : m_ptr(ptr) {}
    inline attr_iterator() = default;
    inline attr_iterator(const attr_iterator&) = default;

    inline const base_t<attr_t>& operator*() const { return m_ptr; }
    inline const attr_t* operator->() { return m_ptr.ptr; }

    inline attr_iterator& operator++() { m_ptr.ptr++; return *this; }  
    inline attr_iterator& operator--() { m_ptr.ptr--; return *this; }  

    inline attr_iterator operator++(int) { attr_iterator tmp = *this; ++(*this); return tmp; }
    inline attr_iterator operator--(int) { attr_iterator tmp = *this; --(*this); return tmp; }

    inline friend bool operator== (const attr_iterator& a, const attr_iterator& b) { return (a.m_ptr.base == b.m_ptr.base) && (a.m_ptr.ptr == b.m_ptr.ptr); };
    inline friend bool operator!= (const attr_iterator& a, const attr_iterator& b) { return (a.m_ptr.base != b.m_ptr.base) || (a.m_ptr.ptr != b.m_ptr.ptr); }; 

    private:
    base_t<attr_t> m_ptr;
};

static_assert(std::bidirectional_iterator<attr_iterator>);
static_assert(std::bidirectional_iterator<node_iterator>);

template <typename T>
inline auto base_t<T>::attrs() const{
    struct self{

        attr_iterator begin() const {return  base_t<attr_t>{*base.base , (*base.attrs_range()).first};}
        attr_iterator end() const {return  base_t<attr_t>{*base.base, (*base.attrs_range()).second};}

        self(const base_t& b):base(b){}

        private:
            base_t base;
    };

    return self(*this);
}

template <typename T>
inline auto base_t<T>::children() const{
    struct self{
        node_iterator begin() const {return base_t<unknown_t>{*base.base, (const unknown_t*)(*base.children_range()).first};}
        node_iterator end() const {return base_t<unknown_t>{*base.base, (const unknown_t*)(*base.children_range()).second};}

        self(const base_t& b):base(b){}

        private:
            base_t base;
    };

    return self(*this);
}

template <typename T>
inline auto base_t<T>::attrs(auto filter) const{ return attrs() | filter ; }

template <typename T>
inline auto base_t<T>::children(auto filter) const{ return children() | filter ; }


}


}


/**
 * @brief Formatter for VS_XML_NS::wrp::sv, the custom stringview used by this library for wrapper trees/documents
 */
template <>
struct std::formatter<VS_XML_NS::wrp::sv, char> : std::formatter<std::string_view, char> {
    template <typename FormatContext>
    auto format(const VS_XML_NS::wrp::sv& value, FormatContext& ctx) const {
        return std::formatter<std::string_view, char>::format((std::string_view)(value), ctx);
    }
};