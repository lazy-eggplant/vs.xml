#pragma once

#include "tree.hpp"
#include "vs-xml/impl.hpp"
#include <iterator>

namespace VS_XML_NS{

namespace wrp{

struct node_iterator;
struct attr_iterator;



template <typename T>
struct base_t{
    private:
        const TreeRaw* base;
        const T*       ptr;
    
        base_t(const TreeRaw& base, const T* ptr):base(&base),ptr(ptr){}
        base_t(base_t p, const T* ptr):base(p.base),ptr(ptr){}
        base_t() = default;

        friend struct VS_XML_NS::Tree;
        template <typename W>
        friend struct base_t;

        friend struct node_iterator;
        friend struct attr_iterator;
    public:
    
    base_t(const base_t& ) = default;

    inline explicit operator const T*() const  {return ptr;}

    delta_ptr_t portable() const;

    inline std::expected<std::string_view,feature_t> ns() const{auto tmp = ptr->ns(); if(!tmp.has_value())return std::unexpected{tmp.error()}; else return base->rsv(*tmp);}
    inline std::expected<std::string_view,feature_t> name() const{auto tmp = ptr->name(); if(!tmp.has_value())return std::unexpected{tmp.error()}; else return base->rsv(*tmp);}
    inline std::expected<std::string_view,feature_t> value() const{auto tmp = ptr->value(); if(!tmp.has_value())return std::unexpected{tmp.error()}; else return base->rsv(*tmp);}

    inline std::expected<std::pair<base_t<unknown_t>, base_t<unknown_t>>,feature_t> children_range() const{
        auto tmp = ptr->children_range();
        if(!tmp.has_value())return std::unexpected{tmp.error()};
        else return std::pair{base_t<unknown_t>(*base,tmp->first), base_t<unknown_t>(*base,tmp->second)};
    }
    inline std::expected<std::pair<const attr_t*, const attr_t*>,feature_t> attrs_range() const{return ptr->attrs_range();}

    inline base_t<element_t> parent() const {return {base,ptr->parent()};}
    inline base_t<unknown_t> prev() const {return {base,ptr->prev()};}
    inline base_t<unknown_t> next() const {return {base,ptr->next()};}

    inline bool has_parent() const{return ptr->has_parent();}
    inline bool has_prev() const{return ptr->has_prev();}
    inline bool has_next() const{return ptr->has_next();}

    inline auto attrs() const;
    inline auto children() const;
};


template <>
struct base_t<attr_t>{
    private:
        const TreeRaw* base;
        const attr_t*  ptr;
    
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

    delta_ptr_t portable() const;

    inline std::expected<std::string_view,feature_t> ns() const{auto tmp = ptr->ns(); if(!tmp.has_value())return std::unexpected{tmp.error()}; else return base->rsv(*tmp);}
    inline std::expected<std::string_view,feature_t> name() const{auto tmp = ptr->name(); if(!tmp.has_value())return std::unexpected{tmp.error()}; else return base->rsv(*tmp);}
    inline std::expected<std::string_view,feature_t> value() const{auto tmp = ptr->value(); if(!tmp.has_value())return std::unexpected{tmp.error()}; else return base->rsv(*tmp);}
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
    inline const base_t<unknown_t>& operator->() { return m_ptr; }

    inline node_iterator& operator++() { m_ptr.ptr = m_ptr.ptr->next(); return *this; }  
    inline node_iterator& operator--() { m_ptr.ptr = m_ptr.ptr->prev(); return *this; }  

    inline node_iterator operator++(int) { node_iterator tmp = *this; ++(*this); return tmp; }
    inline node_iterator operator--(int) { node_iterator tmp = *this; --(*this); return tmp; }

    inline friend bool operator== (const node_iterator& a, const node_iterator& b) { return (&a.m_ptr.base == &b.m_ptr.base) && (a.m_ptr.ptr == b.m_ptr.ptr); };
    inline friend bool operator!= (const node_iterator& a, const node_iterator& b) { return (&a.m_ptr.base != &b.m_ptr.base) || (a.m_ptr.ptr != b.m_ptr.ptr); };  


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
    inline const base_t<attr_t>& operator->() { return m_ptr; }

    inline attr_iterator& operator++() { m_ptr.ptr++; return *this; }  
    inline attr_iterator& operator--() { m_ptr.ptr--; return *this; }  

    inline attr_iterator operator++(int) { attr_iterator tmp = *this; ++(*this); return tmp; }
    inline attr_iterator operator--(int) { attr_iterator tmp = *this; --(*this); return tmp; }

    inline friend bool operator== (const attr_iterator& a, const attr_iterator& b) { return (&a.m_ptr.base == &b.m_ptr.base) && (a.m_ptr.ptr == b.m_ptr.ptr); };
    inline friend bool operator!= (const attr_iterator& a, const attr_iterator& b) { return (&a.m_ptr.base != &b.m_ptr.base) || (a.m_ptr.ptr != b.m_ptr.ptr); }; 


    private:
    base_t<attr_t> m_ptr;
};

static_assert(std::bidirectional_iterator<attr_iterator>);

template <typename T>
inline auto base_t<T>::attrs() const{
    struct self{

        attr_iterator begin() const {return  base_t<attr_t>{*base->base, (*base->attrs_range()).first};}
        attr_iterator end() const {return  base_t<attr_t>{*base->base, (*base->attrs_range()).second};}

        self(const base_t& b):base(&b){}

        private:
            const base_t* base;
    };

    return self(*this);
}


template <typename T>
inline auto base_t<T>::children() const{
    struct self{
        node_iterator begin() const {std::printf("%p %p --start\n",base->base,(const unknown_t*)(*base->children_range()).first);return base_t<unknown_t>{*base->base, (const unknown_t*)(*base->children_range()).first};}
        node_iterator end() const {std::printf("%p %p --end\n",base->base,(const unknown_t*)(*base->children_range()).second);return base_t<unknown_t>{*base->base, (const unknown_t*)(*base->children_range()).second};}

        self(const base_t& b):base(&b){}

        private:
            const base_t* base;
    };

    return self(*this);
}

}


}
