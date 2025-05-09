#pragma once

/**
 * @file impl.hpp
 * @author karurochari
 * @brief Implementation of the node logic
 * @date 2025-05-01
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <cstddef>
#include <cassert>
#include <cstdint>

#include <iterator>
#include <expected>

#include <string>
#include <string_view>

#include <format>
#include <utility>

#include "commons.hpp"
#include "serializer.hpp"

namespace VS_XML_NS{

template <typename T>
struct base_t{
    protected:
    type_t _type = T::deftype();

    public:
    typedef T base;

    type_t type() const {return _type;};

    std::expected<sv,feature_t> ns() const {return static_cast<const T*>(this)->ns();}
    std::expected<sv,feature_t> name() const {return static_cast<const T*>(this)->name();}
    std::expected<sv,feature_t> value() const {return static_cast<const T*>(this)->value();}

    std::expected<std::pair<const unknown_t*, const unknown_t*>,feature_t> children() const {return static_cast<const T*>(this)->children();}
    std::expected<std::pair<const attr_t*, const attr_t*>,feature_t> attrs() const {return static_cast<const T*>(this)->attrs();}

    const node_t* parent() const {return static_cast<const T*>(this)->parent();}
    const unknown_t* prev() const {return static_cast<const T*>(this)->prev();}
    const unknown_t* next() const {return static_cast<const T*>(this)->next();}

    bool has_parent() const {return static_cast<const T*>(this)->has_parent();}
    bool has_prev() const {return static_cast<const T*>(this)->has_prev();}
    bool has_next() const {return static_cast<const T*>(this)->has_next();}

    std::string path() const {
        return std::format("{}/{}",parent()!=nullptr?parent()->path():"",static_cast<const T*>(this)->path_h());
    }

    auto children_fwd() const;
    auto attrs_fwd() const;

    friend Builder;
    friend Tree;
};

struct attr_t{
    private:
    sv _ns;
    sv _name;
    sv _value;

    public:

    inline attr_t(void* offset, std::string_view _ns, std::string_view _name, std::string_view _value):
        _ns(offset,serialize::validate_xml_label(_ns)),
        _name(offset,serialize::validate_xml_label(_name)),
        _value(offset,_value){}

    inline std::expected<sv,feature_t> ns() const {return _ns;}
    inline std::expected<sv,feature_t> name() const {return _name;}
    inline std::expected<sv,feature_t> value() const {return _value;}
};

struct node_t : base_t<node_t>{
    private:
    delta_ptr_t _parent;
    delta_ptr_t _prev;
    delta_ptr_t _next;

    xml_size_t  _size;

    xml_count_t  attrs_count;

    sv _ns;
    sv _name;

    attr_t _attrs[];

    inline node_t(void* offset, node_t* _parent, std::string_view _ns, std::string_view _name):
        _ns(offset,serialize::validate_xml_label(_ns)),
        _name(offset,serialize::validate_xml_label(_name))
    {
        set_parent(_parent);
        _size=0;
        attrs_count=0;
    }

    inline void set_parent(node_t* parent){auto tmp=(uint8_t*)parent-(uint8_t*)this;_parent=tmp;xml_assert((std::ptrdiff_t)_parent==tmp);}
    inline void set_prev(unknown_t* prev){auto tmp=(uint8_t*)prev-(uint8_t*)this;_prev=tmp;xml_assert((std::ptrdiff_t)_prev==tmp);}
    inline void set_next(unknown_t* next){auto tmp=(uint8_t*)next-(uint8_t*)this;_next=tmp;xml_assert((std::ptrdiff_t)_next==tmp);}

    //Unsafe, not boundary checked.
    inline attr_t& get_attr(xml_count_t a) const{return (attr_t&)_attrs[a];}

    public:

    using base_t::type;
    
    static inline type_t deftype() {return type_t::NODE;};
    inline std::expected<sv,feature_t> ns() const {return _ns;}
    inline std::expected<sv,feature_t> name() const {return _name;}
    inline std::expected<sv,feature_t> value() const {return std::unexpected(feature_t::NOT_SUPPORTED);}

    inline std::expected<std::pair<const unknown_t*, const unknown_t*>,feature_t> children() const {
        return std::pair{
            (const unknown_t*)((const uint8_t*)this+sizeof(node_t)+sizeof(attr_t)*attrs_count),
            (const unknown_t*)((const uint8_t*)this+_size)
        };
    }
    inline std::expected<std::pair<const attr_t*, const attr_t*>,feature_t> attrs() const {
        return std::pair{
            (const attr_t*)((const uint8_t*)this+sizeof(node_t)),
            (const attr_t*)((const uint8_t*)this+sizeof(node_t)+sizeof(attr_t)*attrs_count)
        };
    }

    inline const node_t* parent() const {
        if(_parent==0)return nullptr;
        return (const node_t*)((const uint8_t*)this+_parent);
    }
    inline const unknown_t* prev() const {
        if(_prev==0)return nullptr;  //TODO: check this one
        return (const unknown_t*)((const uint8_t*)this+_prev);
    }
    inline const unknown_t* next() const {
        if(_next==0)return (const unknown_t*) (parent()->_size+_parent+(const uint8_t*)this); 
        return (const unknown_t*)((const uint8_t*)this+_next);
    }

    inline bool has_parent() const {return _parent!=0;}
    inline bool has_prev() const {return _prev!=0;}
    inline bool has_next() const {return _next!=0;}

    /*
    inline std::string path_h() const {
        return std::format("{}{}{}", _ns, _ns==""?"":":", _name);
    }
    */
    
    friend Builder;
    friend Tree;
    friend unknown_t;
};


template<typename T>
struct leaf_t : base_t<T>{
    private:
    delta_ptr_t _parent;
    delta_ptr_t _prev;
    //Not needed. Can be statically determined by its size and the children information of the parent.
    //delta_ptr_t _next; 

    sv _value;

    inline void set_parent(node_t* parent){auto tmp=(uint8_t*)parent-(uint8_t*)this;_parent=tmp;xml_assert((std::ptrdiff_t)_parent==tmp);}
    inline void set_prev(unknown_t* prev){auto tmp=(uint8_t*)prev-(uint8_t*)this;_prev=tmp;xml_assert((std::ptrdiff_t)_prev==tmp);}
    inline void set_next(unknown_t* next){/*not needed*/}


    protected:
    
    leaf_t(void* offset, std::string_view value):_value(offset,value){}

    public:

    using base_t<T>::type;

    inline std::expected<sv,feature_t> ns() const {return std::unexpected(feature_t::NOT_SUPPORTED);}
    inline std::expected<sv,feature_t> name() const {return std::unexpected(feature_t::NOT_SUPPORTED);}
    inline std::expected<sv,feature_t> value() const {return _value;}

    inline std::expected<std::pair<const unknown_t*, const unknown_t*>,feature_t> children() const {return std::unexpected(feature_t::NOT_SUPPORTED);}
    inline std::expected<std::pair<const attr_t*, const attr_t*>,feature_t> attrs() const {return std::unexpected(feature_t::NOT_SUPPORTED);}

    inline const node_t* parent() const {return (const node_t*)((const uint8_t*)this+_parent);}
    inline const unknown_t*prev() const {return (const unknown_t*)((const uint8_t*)this+_prev);}
    inline const unknown_t* next() const {return (const unknown_t*)((const uint8_t*)this+sizeof(leaf_t));}

    inline bool has_parent() const {return _parent!=0;}
    inline bool has_prev() const {return _prev!=0;}
    inline bool has_next() const {return has_parent() && (next()<(parent()->children())->second)!=0;}   //TODO:check

    friend Builder;
    friend Tree;
    friend unknown_t;
};

struct comment_t : leaf_t<comment_t>{
    comment_t(void* offset, std::string_view value):leaf_t(offset, value){}
    static inline type_t deftype() {return type_t::COMMENT;};

    inline std::string path_h() const { return std::format("#comment"); }

    friend Builder;
    friend Tree;
};

struct cdata_t : leaf_t<cdata_t>{
    cdata_t(void* offset, std::string_view value):leaf_t(offset, value){}
    static inline type_t deftype() {return type_t::CDATA;};

    inline std::string path_h() const { return std::format("#cdata"); }

    friend Builder;
    friend Tree;
};

struct text_t : leaf_t<text_t>{
    text_t(void* offset, std::string_view value):leaf_t(offset, value){}
    static inline type_t deftype() {return type_t::TEXT;};

    inline std::string path_h() const { return std::format("#text"); }
    
    friend Builder;
    friend Tree;
};

struct proc_t : leaf_t<proc_t>{
    proc_t(void* offset, std::string_view value):leaf_t(offset, value){}
    static inline type_t deftype() {return type_t::PROC;};

    inline std::string path_h() const { return std::format("#proc"); }

    friend Builder;
    friend Tree;
};

struct inject_t : leaf_t<inject_t>{
    inject_t(void* offset, std::string_view value):leaf_t(offset, value){}
    static inline type_t deftype() {return type_t::INJECT;};

    inline std::string path_h() const { return std::format("#leaf"); }

    friend Builder;
    friend Tree;
};

#define DISPATCH(X) \
if (type() == type_t::NODE) return ((node_t*)this) -> X;\
else if (type() == type_t::TEXT) return ((text_t*)this)-> X;\
else if (type() == type_t::COMMENT) return ((comment_t*)this)-> X;\
else if (type() == type_t::PROC) return ((proc_t*)this)-> X;\
else if (type() == type_t::CDATA) return ((cdata_t*)this)-> X;\
else if (type() == type_t::INJECT) return ((inject_t*)this)-> X;\
else{\
    xml_assert(false,"Invalid XML thing type");\
    std::unreachable();\
}

#define CDISPATCH(X) \
if (type() == type_t::NODE) return ((const node_t*)this) -> X;\
else if (type() == type_t::TEXT) return ((const text_t*)this)-> X;\
else if (type() == type_t::COMMENT) return ((const comment_t*)this)-> X;\
else if (type() == type_t::PROC) return ((const proc_t*)this)-> X;\
else if (type() == type_t::CDATA) return ((const cdata_t*)this)-> X;\
else if (type() == type_t::INJECT) return ((const inject_t*)this)-> X;\
else{\
    xml_assert(false,"Invalid XML thing type");\
    std::unreachable();\
}

struct unknown_t : base_t<unknown_t>{
    private:

    void set_parent(node_t* parent){DISPATCH(set_parent(parent));}
    void set_prev(unknown_t* prev){DISPATCH(set_prev(prev));}
    void set_next(unknown_t* next){DISPATCH(set_next(next));}

    public:

    static inline type_t deftype() {return type_t::UNKNOWN;};

    std::expected<sv,feature_t> ns() const {CDISPATCH(ns());}
    std::expected<sv,feature_t> name() const {CDISPATCH(name());}
    std::expected<sv,feature_t> value() const {CDISPATCH(value());}

    std::expected<std::pair<const unknown_t*, const unknown_t*>,feature_t> children() const {CDISPATCH(children());}
    std::expected<std::pair<const attr_t*, const attr_t*>,feature_t> attrs() const {CDISPATCH(attrs());}

    const node_t* parent() const {CDISPATCH(parent());}
    const unknown_t* prev() const {CDISPATCH(prev());}
    const unknown_t* next() const {CDISPATCH(next());}

    inline bool has_parent() const {CDISPATCH(has_parent());}
    inline bool has_prev() const {CDISPATCH(has_prev());}
    inline bool has_next() const {CDISPATCH(has_next());}

    friend Builder;
    friend Tree;
};

#undef DISPATCH
#undef CDISPATCH

static_assert(thing_i<node_t>);
static_assert(thing_i<comment_t>);
static_assert(thing_i<cdata_t>);
static_assert(thing_i<text_t>);
static_assert(thing_i<proc_t>);
static_assert(thing_i<inject_t>);
static_assert(thing_i<unknown_t>);

struct node_iterator{
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = const unknown_t;
    using pointer           = const value_type*;
    using reference         = const value_type&;

    inline  node_iterator(pointer ptr) : m_ptr(ptr) {}


    inline  reference operator*() const { return *m_ptr; }
    inline pointer operator->() { return m_ptr; }

    inline node_iterator& operator++() { m_ptr=m_ptr->next(); return *this; }  
    inline node_iterator& operator--() { m_ptr=m_ptr->prev(); return *this; }  

    inline node_iterator operator++(int) { node_iterator tmp = *this; ++(*this); return tmp; }
    inline node_iterator operator--(int) { node_iterator tmp = *this; --(*this); return tmp; }

    inline friend bool operator== (const node_iterator& a, const node_iterator& b) { return a.m_ptr == b.m_ptr; };
    inline friend bool operator!= (const node_iterator& a, const node_iterator& b) { return a.m_ptr != b.m_ptr; };  


    private:
    pointer m_ptr;
};

//TODO: it should be  std::random_access_iterator_tag;
struct attr_iterator{
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = const attr_t;
    using pointer           = const value_type*;
    using reference         = const value_type&;

    inline attr_iterator(pointer ptr) : m_ptr(ptr) {}


    inline reference operator*() const { return *m_ptr; }
    inline pointer operator->() { return m_ptr; }

    inline attr_iterator& operator++() { m_ptr++; return *this; }  
    inline attr_iterator& operator--() { m_ptr--; return *this; }  

    inline attr_iterator operator++(int) { attr_iterator tmp = *this; ++(*this); return tmp; }
    inline attr_iterator operator--(int) { attr_iterator tmp = *this; --(*this); return tmp; }

    inline friend bool operator== (const attr_iterator& a, const attr_iterator& b) { return a.m_ptr == b.m_ptr; };
    inline friend bool operator!= (const attr_iterator& a, const attr_iterator& b) { return a.m_ptr != b.m_ptr; };  


    private:
    pointer m_ptr;
};

template <typename T>
inline auto base_t<T>::children_fwd() const{

    struct self{
        node_iterator begin() const {return (*base.children()).first;}
        node_iterator end() const {return (*base.children()).second;}

        self(const base_t& b):base(b){}

        private:
            const base_t& base;
    };

    return self(*this);
}

template <typename T>
inline auto base_t<T>::attrs_fwd() const{

    struct self{
        attr_iterator begin() const {return (*base.attrs()).first;}
        attr_iterator end() const {return (*base.attrs()).second;}

        self(const base_t& b):base(b){}

        private:
            const base_t& base;
    };

    return self(*this);
}



}