#pragma once

#include "tree.hpp"
#include "vs-xml/impl.hpp"
#include <iterator>

namespace VS_XML_NS{

namespace details{

struct wrp_node_iterator;
struct wrp_attr_iterator;



template <typename T>
struct wrp_base_t{
    private:
        const Tree&       base;
        const T*          ptr;
    
        wrp_base_t(const Tree& base, const T* ptr):base(base),ptr(ptr){}
        wrp_base_t(wrp_base_t p, const T* ptr):base(p.base),ptr(ptr){}

        friend struct xml::WrpTree;
        template <typename W>
        friend struct wrp_base_t;

        friend struct wrp_node_iterator;
        friend struct wrp_attr_iterator;
    public:

    inline explicit operator const T*() const  {return ptr;}

    delta_ptr_t portable() const;

    inline std::expected<std::string_view,feature_t> ns() const{auto tmp = ptr->ns(); if(!tmp.has_value())return std::unexpected{tmp.error()}; else return base.rsv(*tmp);}
    inline std::expected<std::string_view,feature_t> name() const{auto tmp = ptr->name(); if(!tmp.has_value())return std::unexpected{tmp.error()}; else return base.rsv(*tmp);}
    inline std::expected<std::string_view,feature_t> value() const{auto tmp = ptr->value(); if(!tmp.has_value())return std::unexpected{tmp.error()}; else return base.rsv(*tmp);}

    inline std::expected<std::pair<wrp_base_t<unknown_t>, wrp_base_t<unknown_t>>,feature_t> children_range() const{
        auto tmp = ptr->children_range();
        if(!tmp.has_value())return std::unexpected{tmp.error()};
        else return std::pair{wrp_base_t<unknown_t>{base,tmp->first}, wrp_base_t<unknown_t>{base,tmp->second}};
    }
    inline std::expected<std::pair<const attr_t*, const attr_t*>,feature_t> attrs_range() const{return ptr->attrs_range();}

    inline wrp_base_t<element_t> parent() const {return {base,ptr->parent()};}
    inline wrp_base_t<unknown_t> prev() const {return {base,ptr->prev()};}
    inline wrp_base_t<unknown_t> next() const {return {base,ptr->next()};}

    inline bool has_parent() const{return ptr->has_parent();}
    inline bool has_prev() const{return ptr->has_prev();}
    inline bool has_next() const{return ptr->has_next();}

    inline auto attrs() const;
    inline auto children() const;
};


template <>
struct wrp_base_t<attr_t>{
    private:
        const Tree&       base;
        const attr_t*          ptr;
    
        wrp_base_t(const Tree& base, const attr_t* ptr):base(base),ptr(ptr){}
        wrp_base_t(wrp_base_t p, const attr_t* ptr):base(p.base),ptr(ptr){}

        friend struct WrpTree;
        template <typename W>
        friend struct wrp_base_t;

        friend struct wrp_node_iterator;
        friend struct wrp_attr_iterator;
    public:

    inline explicit operator const attr_t*() const  {return ptr;}

    delta_ptr_t portable() const;

    inline std::expected<std::string_view,feature_t> ns() const{auto tmp = ptr->ns(); if(!tmp.has_value())return std::unexpected{tmp.error()}; else return base.rsv(*tmp);}
    inline std::expected<std::string_view,feature_t> name() const{auto tmp = ptr->name(); if(!tmp.has_value())return std::unexpected{tmp.error()}; else return base.rsv(*tmp);}
    inline std::expected<std::string_view,feature_t> value() const{auto tmp = ptr->value(); if(!tmp.has_value())return std::unexpected{tmp.error()}; else return base.rsv(*tmp);}
};


struct wrp_node_iterator{
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = const unknown_t;
    using pointer           = wrp_base_t<unknown_t>;
    using reference         = wrp_base_t<unknown_t>;

    inline wrp_node_iterator(wrp_base_t<unknown_t> ptr) : m_ptr(ptr) {}

    inline const wrp_base_t<unknown_t>& operator*() const { return m_ptr; }
    inline wrp_base_t<unknown_t> operator->() { return m_ptr; }

    inline wrp_node_iterator& operator++() { m_ptr.ptr = m_ptr.ptr->next(); return *this; }  
    inline wrp_node_iterator& operator--() { m_ptr.ptr = m_ptr.ptr->prev(); return *this; }  

    inline wrp_node_iterator operator++(int) { wrp_node_iterator tmp = *this; ++(*this); return tmp; }
    inline wrp_node_iterator operator--(int) { wrp_node_iterator tmp = *this; --(*this); return tmp; }

    inline friend bool operator== (const wrp_node_iterator& a, const wrp_node_iterator& b) { return (&a.m_ptr.base == &b.m_ptr.base) && (a.m_ptr.ptr == b.m_ptr.ptr); };
    inline friend bool operator!= (const wrp_node_iterator& a, const wrp_node_iterator& b) { return (&a.m_ptr.base != &b.m_ptr.base) || (a.m_ptr.ptr != b.m_ptr.ptr); };  


    private:
    wrp_base_t<unknown_t> m_ptr;
};

//TODO: it should be  std::random_access_iterator_tag;
struct wrp_attr_iterator{
    using iterator_category = std::bidirectional_iterator_tag; 
    using difference_type   = std::ptrdiff_t;
    using value_type        = const attr_t;
    using pointer           = wrp_base_t<attr_t>;
    using reference         = wrp_base_t<attr_t>;

    inline wrp_attr_iterator(wrp_base_t<attr_t> ptr) : m_ptr(ptr) {}


    inline const wrp_base_t<attr_t>& operator*() const { return m_ptr; }
    inline wrp_base_t<attr_t> operator->() { return m_ptr; }

    inline wrp_attr_iterator& operator++() { m_ptr.ptr++; return *this; }  
    inline wrp_attr_iterator& operator--() { m_ptr.ptr--; return *this; }  

    inline wrp_attr_iterator operator++(int) { wrp_attr_iterator tmp = *this; ++(*this); return tmp; }
    inline wrp_attr_iterator operator--(int) { wrp_attr_iterator tmp = *this; --(*this); return tmp; }

    inline friend bool operator== (const wrp_attr_iterator& a, const wrp_attr_iterator& b) { return (&a.m_ptr.base == &b.m_ptr.base) && (a.m_ptr.ptr == b.m_ptr.ptr); };
    inline friend bool operator!= (const wrp_attr_iterator& a, const wrp_attr_iterator& b) { return (&a.m_ptr.base != &b.m_ptr.base) || (a.m_ptr.ptr != b.m_ptr.ptr); }; 


    private:
    wrp_base_t<attr_t> m_ptr;
};

template <typename T>
inline auto wrp_base_t<T>::attrs() const{
    struct self{
        wrp_attr_iterator begin() const {return  wrp_base_t<attr_t>{base.base, (*base.attrs_range()).first};}
        wrp_attr_iterator end() const {return  wrp_base_t<attr_t>{base.base, (*base.attrs_range()).second};}

        self(const wrp_base_t& b):base(b){}

        private:
            const wrp_base_t& base;
    };

    return self(*this);
}

template <typename T>
inline auto wrp_base_t<T>::children() const{
    struct self{
        wrp_node_iterator begin() const {return wrp_base_t<unknown_t>{base.base, (const unknown_t*)(*base.children_range()).first};}
        wrp_node_iterator end() const {return wrp_base_t<unknown_t>{base.base, (const unknown_t*)(*base.children_range()).second};}

        self(const wrp_base_t& b):base(b){}

        private:
            const wrp_base_t& base;
    };

    return self(*this);
}

}

struct WrpTree : Tree{
    private:
    using Tree::rsv;
    using Tree::clone;
    using Tree::root;

    public:
    inline WrpTree(Tree&& ref):Tree(std::move(ref)){}
    inline WrpTree(const Tree&& ref):Tree(std::move(ref)){}

    inline const WrpTree slice(const element_t* ref=nullptr) const{return Tree::slice(ref);}
    inline WrpTree clone(const element_t* ref=nullptr, bool reduce=true) const{return Tree::clone(ref,reduce);}

    details::wrp_base_t<element_t> root() const;

    inline Tree& downgrade(){return *this;}
};

}