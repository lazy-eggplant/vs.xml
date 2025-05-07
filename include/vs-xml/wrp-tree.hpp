#pragma once

#include "tree.hpp"
#include "vs-xml/impl.hpp"

namespace xml{

struct wrp_node_iterator{
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = const unknown_t;
    using pointer           = const value_type*;
    using reference         = const value_type&;

    inline wrp_node_iterator(pointer ptr) : m_ptr(ptr) {}

    inline reference operator*() const { return *m_ptr; }
    inline pointer operator->() { return m_ptr; }

    inline wrp_node_iterator& operator++() { m_ptr=m_ptr->next(); return *this; }  
    inline wrp_node_iterator& operator--() { m_ptr=m_ptr->prev(); return *this; }  

    inline wrp_node_iterator operator++(int) { wrp_node_iterator tmp = *this; ++(*this); return tmp; }
    inline wrp_node_iterator operator--(int) { wrp_node_iterator tmp = *this; --(*this); return tmp; }

    inline friend bool operator== (const wrp_node_iterator& a, const wrp_node_iterator& b) { return a.m_ptr == b.m_ptr; };
    inline friend bool operator!= (const wrp_node_iterator& a, const wrp_node_iterator& b) { return a.m_ptr != b.m_ptr; };  


    private:
    pointer m_ptr;
};

struct wrp_attr_iterator{
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = const attr_t;
    using pointer           = const value_type*;
    using reference         = const value_type&;

    inline wrp_attr_iterator(pointer ptr) : m_ptr(ptr) {}


    inline reference operator*() const { return *m_ptr; }
    inline pointer operator->() { return m_ptr; }

    inline wrp_attr_iterator& operator++() { m_ptr++; return *this; }  
    inline wrp_attr_iterator& operator--() { m_ptr--; return *this; }  

    inline wrp_attr_iterator operator++(int) { wrp_attr_iterator tmp = *this; ++(*this); return tmp; }
    inline wrp_attr_iterator operator--(int) { wrp_attr_iterator tmp = *this; --(*this); return tmp; }

    inline friend bool operator== (const wrp_attr_iterator& a, const wrp_attr_iterator& b) { return a.m_ptr == b.m_ptr; };
    inline friend bool operator!= (const wrp_attr_iterator& a, const wrp_attr_iterator& b) { return a.m_ptr != b.m_ptr; };  


    private:
    pointer m_ptr;
};


template <thing_i T>
struct wrp_base_t{
    private:
        const Tree&       base;
        const T*          ptr;
    
        wrp_base_t(const Tree& base, const T* ptr):base(base),ptr(ptr){}
        wrp_base_t(wrp_base_t p, const T* ptr):base(p.base),ptr(ptr){}

        friend struct WrpTree;
        template <thing_i W>
        friend struct wrp_base_t;
    public:

    constexpr inline operator const T*() const {return ptr;}

    constexpr delta_ptr_t portable() const;

    inline constexpr std::expected<std::string_view,feature_t> ns() const{auto tmp = ptr->ns(); if(!tmp.has_value())return std::unexpected{tmp.error()}; else return base.rsv(*tmp);}
    inline constexpr std::expected<std::string_view,feature_t> name() const{auto tmp = ptr->name(); if(!tmp.has_value())return std::unexpected{tmp.error()}; else return base.rsv(*tmp);}
    inline constexpr std::expected<std::string_view,feature_t> value() const{auto tmp = ptr->value(); if(!tmp.has_value())return std::unexpected{tmp.error()}; else return base.rsv(*tmp);}

    inline constexpr std::expected<std::pair<wrp_base_t<unknown_t>, wrp_base_t<unknown_t>>,feature_t> children() const;
    inline constexpr std::expected<std::pair<const attr_t*, const attr_t*>,feature_t> attrs() const{return ptr->attrs();}

    inline constexpr wrp_base_t<node_t> parent() const {return {base,ptr->parent()};}
    inline constexpr wrp_base_t<unknown_t> prev() const {return {base,ptr->prev()};}
    inline constexpr wrp_base_t<unknown_t> next() const {return {base,ptr->next()};}

    inline constexpr bool has_parent() const{return ptr->has_parent();}
    inline constexpr bool has_prev() const{return ptr->has_prev();}
    inline constexpr bool has_next() const{return ptr->has_next();}

    inline constexpr auto attrs_fwd() const{
        struct self{
            wrp_attr_iterator begin() const {return (*base.attrs()).first;}
            wrp_attr_iterator end() const {return (*base.attrs()).second;}
    
            self(const wrp_base_t& b):base(b){}
    
            private:
                const wrp_base_t& base;
        };

        return self(*this);
    }

    inline constexpr auto children_fwd() const{
        struct self{
            wrp_node_iterator begin() const {return (const unknown_t*) ((*base.children()).first);}
            wrp_node_iterator end() const {return (const unknown_t*) ((*base.children()).second);}
    
            self(const wrp_base_t& b):base(b){}
    
            private:
                const wrp_base_t& base;
        };
    
        return self(*this);
    }
};

struct WrpTree : Tree{
    private:
    using Tree::rsv;
    using Tree::clone;
    using Tree::root;

    public:
    inline WrpTree(Tree&& ref):Tree(std::move(ref)){}

    inline WrpTree clone(const node_t* ref=nullptr, bool reduce=true) const{return Tree::clone(ref,reduce);}

    wrp_base_t<node_t> root() const;

};

}