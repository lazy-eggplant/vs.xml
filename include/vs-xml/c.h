/**
 * @file c.h
 * @author karurochari
 * @brief C bindings for vs.xml.
 * @date 2026-09-11
 *
 * @copyright Copyright (c) 2025
 *
 * @details
 * A single-header C API. The implementation is compiled once by defining
 * `VSXML_CXX_IMPL` in exactly one C++ translation unit before including this
 * header (see `lib/c.cpp`). All symbols are namespaced through `VSXML_NS`.
 *
 * Handles returned by this API are owned by the caller and must be released
 * with the matching `_destroy` function. String views returned by node accessors
 * point into the document storage and stay valid only while that document lives.
 */

#pragma once

#ifndef VSXML_NS
#   define VSXML_NS vsxml
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define VSXML$_1(ns, x) ns ## _ ## x
#define VSXML$_2(ns, x) VSXML$_1(ns, x)
#define VSXML$(x) VSXML$_2(VSXML_NS, x)

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    VSXML$(Ok) = 0,
    VSXML$(ErrorGeneric) = 1,
    VSXML$(ErrorInvalidArgument) = 2,
    VSXML$(ErrorParse) = 3,
    VSXML$(ErrorIO) = 4,
    VSXML$(ErrorAllocation) = 5,
    VSXML$(ErrorNotImplemented) = 6
} VSXML$(error_t);

typedef enum {
    VSXML$(type_unknown) = 0,
    VSXML$(type_element) = 1,
    VSXML$(type_attr) = 2,
    VSXML$(type_text) = 3,
    VSXML$(type_cdata) = 4,
    VSXML$(type_comment) = 5,
    VSXML$(type_proc) = 6,
    VSXML$(type_marker) = 7
} VSXML$(node_type_t);

/** A non-owning string view. `data` may be NULL when `len` is 0. */
typedef struct {
    const char* data;
    size_t len;
} VSXML$(str_t);

/** A node handle. Zero-initialize for a null handle. */
typedef struct {
    const void* ctx; /* owning document */
    const void* ptr; /* raw node pointer */
} VSXML$(node_t);

typedef struct VSXML$(document) VSXML$(document);
typedef struct VSXML$(builder)  VSXML$(builder);
typedef struct VSXML$(query)    VSXML$(query);

/* ------------------------------------------------------------------ */
/* Documents                                                          */
/* ------------------------------------------------------------------ */

VSXML$(document)* VSXML$(parse)(const char* xml, size_t len, VSXML$(error_t)* err);
VSXML$(document)* VSXML$(document_load)(const uint8_t* data, size_t len, VSXML$(error_t)* err);
VSXML$(document)* VSXML$(document_load_file)(const char* path, VSXML$(error_t)* err);
int VSXML$(document_save_file)(const VSXML$(document)* doc, const char* path);
size_t VSXML$(document_print)(const VSXML$(document)* doc, char* buf, size_t cap);
VSXML$(node_t) VSXML$(document_root)(const VSXML$(document)* doc);
void VSXML$(document_destroy)(VSXML$(document)* doc);

/* ------------------------------------------------------------------ */
/* Building                                                           */
/* ------------------------------------------------------------------ */

VSXML$(builder)* VSXML$(builder_create)(VSXML$(error_t)* err);
int VSXML$(builder_xml)(VSXML$(builder)* b);
int VSXML$(builder_begin)(VSXML$(builder)* b, const char* name, const char* ns);
int VSXML$(builder_end)(VSXML$(builder)* b);
int VSXML$(builder_attr)(VSXML$(builder)* b, const char* name, const char* value, const char* ns);
int VSXML$(builder_text)(VSXML$(builder)* b, const char* value);
int VSXML$(builder_cdata)(VSXML$(builder)* b, const char* value);
int VSXML$(builder_comment)(VSXML$(builder)* b, const char* value);
int VSXML$(builder_proc)(VSXML$(builder)* b, const char* value);
VSXML$(document)* VSXML$(builder_close)(VSXML$(builder)* b, VSXML$(error_t)* err);
void VSXML$(builder_destroy)(VSXML$(builder)* b);

/* ------------------------------------------------------------------ */
/* Nodes                                                              */
/* ------------------------------------------------------------------ */

int VSXML$(node_valid)(VSXML$(node_t) n);
int VSXML$(node_type)(VSXML$(node_t) n);
VSXML$(str_t) VSXML$(node_name)(VSXML$(node_t) n);
VSXML$(str_t) VSXML$(node_ns)(VSXML$(node_t) n);
VSXML$(str_t) VSXML$(node_value)(VSXML$(node_t) n);
VSXML$(node_t) VSXML$(node_parent)(VSXML$(node_t) n);
VSXML$(node_t) VSXML$(node_first_child)(VSXML$(node_t) n);
VSXML$(node_t) VSXML$(node_next_sibling)(VSXML$(node_t) n);
size_t VSXML$(node_attr_count)(VSXML$(node_t) n);
int VSXML$(node_attr_at)(VSXML$(node_t) n, size_t idx, VSXML$(str_t)* ns, VSXML$(str_t)* name, VSXML$(str_t)* value);
size_t VSXML$(node_text)(VSXML$(node_t) n, char* buf, size_t cap);

/* ------------------------------------------------------------------ */
/* Queries                                                            */
/* ------------------------------------------------------------------ */

/** Callback invoked per accepted node; return non-zero to stop the traversal. */
typedef int (*VSXML$(query_cb))(VSXML$(node_t) node, void* user);

VSXML$(query)* VSXML$(query_create)(VSXML$(error_t)* err);
int VSXML$(query_accept)(VSXML$(query)* q);
int VSXML$(query_child)(VSXML$(query)* q);
int VSXML$(query_descend)(VSXML$(query)* q);
int VSXML$(query_fork)(VSXML$(query)* q);
int VSXML$(query_match_type)(VSXML$(query)* q, int type);
int VSXML$(query_match_name)(VSXML$(query)* q, const char* name);
int VSXML$(query_match_ns)(VSXML$(query)* q, const char* ns);
int VSXML$(query_match_value)(VSXML$(query)* q, const char* value);
int VSXML$(query_match_text)(VSXML$(query)* q, const char* text);
int VSXML$(query_match_attr)(VSXML$(query)* q, const char* ns, const char* name, const char* value);
int VSXML$(query_each)(const VSXML$(query)* q, VSXML$(node_t) root, VSXML$(query_cb) cb, void* user);
size_t VSXML$(query_count)(const VSXML$(query)* q, VSXML$(node_t) root);
void VSXML$(query_destroy)(VSXML$(query)* q);

#ifdef __cplusplus
}
#endif

/* ================================================================== */
/* C++ implementation                                                 */
/* ================================================================== */
#ifdef VSXML_CXX_IMPL

#include <cstring>
#include <fstream>
#include <iterator>
#include <new>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include <vs-xml/document.hpp>
#include <vs-xml/document-builder.hpp>
#include <vs-xml/parser.hpp>
#include <vs-xml/query.hpp>

using namespace VS_XML_NS;

struct VSXML$(document){
    std::vector<uint8_t> blob;                // owned bytes when loaded from binary
    std::optional<stored::Document> owned;     // owning document when parsed/built
    std::optional<DocumentRaw> view;           // view over `blob`

    const DocumentRaw& raw() const{
        return owned.has_value() ? static_cast<const DocumentRaw&>(*owned) : *view;
    }
};

struct VSXML$(builder){
    DocumentBuilder<builder_config_t{.symbols=builder_config_t::COMPRESS_ALL}> bld;
};

struct VSXML$(query){
    query::query_t q;
};

namespace {

inline void set_err(VSXML$(error_t)* err, VSXML$(error_t) value){
    if(err != nullptr)*err = value;
}

inline VSXML$(str_t) to_str(std::string_view v){
    return VSXML$(str_t){v.data(), v.size()};
}

inline std::optional<wrp::base_t<unknown_t>> node_of(VSXML$(node_t) n){
    if(n.ctx == nullptr || n.ptr == nullptr)return std::nullopt;
    const auto* doc = static_cast<const VSXML$(document)*>(n.ctx);
    return wrp::base_t<unknown_t>::from(doc->raw(), static_cast<const unknown_t*>(n.ptr));
}

template<typename T>
inline VSXML$(node_t) make_node(VSXML$(node_t) base, wrp::base_t<T> n){
    return VSXML$(node_t){base.ctx, static_cast<const T*>(n)};
}

inline query::filter_t to_filter(const char* s){
    return s != nullptr ? query::filter_t{s} : query::any();
}

inline VSXML$(document)* wrap_owned(stored::Document&& doc, VSXML$(error_t)* err){
    auto* out = new (std::nothrow) VSXML$(document);
    if(out == nullptr){
        set_err(err, VSXML$(ErrorAllocation));
        return nullptr;
    }
    out->owned = std::move(doc);
    return out;
}

} // namespace

extern "C" {

VSXML$(document)* VSXML$(parse)(const char* xml, size_t len, VSXML$(error_t)* err){
    if(xml == nullptr){ set_err(err, VSXML$(ErrorInvalidArgument)); return nullptr; }
    std::string buffer(xml, len); // mutable: the parser may unescape in place.
    DocumentBuilder<builder_config_t{.symbols=builder_config_t::COMPRESS_ALL}> bld;
    Parser parser(std::span<char>(buffer.data(), buffer.size()), bld);
    if(auto r = parser.parse(); !r.has_value()){
        set_err(err, VSXML$(ErrorParse));
        return nullptr;
    }
    auto doc = bld.close();
    if(!doc.has_value()){
        set_err(err, VSXML$(ErrorGeneric));
        return nullptr;
    }
    set_err(err, VSXML$(Ok));
    return wrap_owned(std::move(*doc), err);
}

VSXML$(document)* VSXML$(document_load)(const uint8_t* data, size_t len, VSXML$(error_t)* err){
    if(data == nullptr){ set_err(err, VSXML$(ErrorInvalidArgument)); return nullptr; }
    auto* out = new (std::nothrow) VSXML$(document);
    if(out == nullptr){ set_err(err, VSXML$(ErrorAllocation)); return nullptr; }
    out->blob.assign(data, data + len);
    auto view = DocumentRaw::from_binary(std::span<const uint8_t>(out->blob.data(), out->blob.size()));
    if(!view.has_value()){
        delete out;
        set_err(err, VSXML$(ErrorGeneric));
        return nullptr;
    }
    out->view = std::move(*view);
    set_err(err, VSXML$(Ok));
    return out;
}

VSXML$(document)* VSXML$(document_load_file)(const char* path, VSXML$(error_t)* err){
    if(path == nullptr){ set_err(err, VSXML$(ErrorInvalidArgument)); return nullptr; }
    std::ifstream in(path, std::ios::binary);
    if(!in.is_open()){ set_err(err, VSXML$(ErrorIO)); return nullptr; }
    std::vector<uint8_t> blob{
        std::istreambuf_iterator<char>(in),
        std::istreambuf_iterator<char>()
    };
    return VSXML$(document_load)(blob.data(), blob.size(), err);
}

int VSXML$(document_save_file)(const VSXML$(document)* doc, const char* path){
    if(doc == nullptr || path == nullptr)return (int)VSXML$(ErrorInvalidArgument);
    std::ofstream out(path, std::ios::binary);
    if(!out.is_open())return (int)VSXML$(ErrorIO);
    return doc->raw().save_binary(out) ? (int)VSXML$(Ok) : (int)VSXML$(ErrorGeneric);
}

size_t VSXML$(document_print)(const VSXML$(document)* doc, char* buf, size_t cap){
    if(doc == nullptr)return 0;
    std::ostringstream os;
    doc->raw().print(os);
    const std::string s = os.str();
    if(buf != nullptr && cap > 0){
        const size_t n = s.size() < cap - 1 ? s.size() : cap - 1;
        std::memcpy(buf, s.data(), n);
        buf[n] = '\0';
    }
    return s.size();
}

VSXML$(node_t) VSXML$(document_root)(const VSXML$(document)* doc){
    if(doc == nullptr)return VSXML$(node_t){nullptr, nullptr};
    auto tr = doc->raw().tree_root();
    if(tr.has_value())return VSXML$(node_t){doc, (const void*)&**tr};
    return VSXML$(node_t){doc, &doc->raw().root()};
}

void VSXML$(document_destroy)(VSXML$(document)* doc){
    delete doc;
}

VSXML$(builder)* VSXML$(builder_create)(VSXML$(error_t)* err){
    auto* out = new (std::nothrow) VSXML$(builder);
    set_err(err, out != nullptr ? VSXML$(Ok) : VSXML$(ErrorAllocation));
    return out;
}

int VSXML$(builder_xml)(VSXML$(builder)* b){ return b ? (int)b->bld.xml() : (int)VSXML$(ErrorInvalidArgument); }
int VSXML$(builder_begin)(VSXML$(builder)* b, const char* name, const char* ns){
    if(b == nullptr || name == nullptr)return (int)VSXML$(ErrorInvalidArgument);
    return (int)b->bld.begin(name, ns != nullptr ? ns : "");
}
int VSXML$(builder_end)(VSXML$(builder)* b){ return b ? (int)b->bld.end() : (int)VSXML$(ErrorInvalidArgument); }
int VSXML$(builder_attr)(VSXML$(builder)* b, const char* name, const char* value, const char* ns){
    if(b == nullptr || name == nullptr || value == nullptr)return (int)VSXML$(ErrorInvalidArgument);
    return (int)b->bld.attr(name, value, ns != nullptr ? ns : "");
}
int VSXML$(builder_text)(VSXML$(builder)* b, const char* value){
    if(b == nullptr || value == nullptr)return (int)VSXML$(ErrorInvalidArgument);
    return (int)b->bld.text(value);
}
int VSXML$(builder_cdata)(VSXML$(builder)* b, const char* value){
    if(b == nullptr || value == nullptr)return (int)VSXML$(ErrorInvalidArgument);
    return (int)b->bld.cdata(value);
}
int VSXML$(builder_comment)(VSXML$(builder)* b, const char* value){
    if(b == nullptr || value == nullptr)return (int)VSXML$(ErrorInvalidArgument);
    return (int)b->bld.comment(value);
}
int VSXML$(builder_proc)(VSXML$(builder)* b, const char* value){
    if(b == nullptr || value == nullptr)return (int)VSXML$(ErrorInvalidArgument);
    return (int)b->bld.proc(value);
}
VSXML$(document)* VSXML$(builder_close)(VSXML$(builder)* b, VSXML$(error_t)* err){
    if(b == nullptr){ set_err(err, VSXML$(ErrorInvalidArgument)); return nullptr; }
    auto doc = b->bld.close();
    if(!doc.has_value()){ set_err(err, VSXML$(ErrorGeneric)); return nullptr; }
    set_err(err, VSXML$(Ok));
    return wrap_owned(std::move(*doc), err);
}
void VSXML$(builder_destroy)(VSXML$(builder)* b){ delete b; }

int VSXML$(node_valid)(VSXML$(node_t) n){ return node_of(n).has_value() ? 1 : 0; }

int VSXML$(node_type)(VSXML$(node_t) n){
    auto node = node_of(n);
    return node.has_value() ? (int)node->type() : (int)VSXML$(type_unknown);
}

VSXML$(str_t) VSXML$(node_name)(VSXML$(node_t) n){
    auto node = node_of(n);
    if(!node.has_value())return VSXML$(str_t){nullptr, 0};
    auto v = node->name();
    return v.has_value() ? to_str(std::string_view(*v)) : VSXML$(str_t){nullptr, 0};
}

VSXML$(str_t) VSXML$(node_ns)(VSXML$(node_t) n){
    auto node = node_of(n);
    if(!node.has_value())return VSXML$(str_t){nullptr, 0};
    auto v = node->ns();
    return v.has_value() ? to_str(std::string_view(*v)) : VSXML$(str_t){nullptr, 0};
}

VSXML$(str_t) VSXML$(node_value)(VSXML$(node_t) n){
    auto node = node_of(n);
    if(!node.has_value())return VSXML$(str_t){nullptr, 0};
    auto v = node->value();
    return v.has_value() ? to_str(std::string_view(*v)) : VSXML$(str_t){nullptr, 0};
}

VSXML$(node_t) VSXML$(node_parent)(VSXML$(node_t) n){
    auto node = node_of(n);
    if(!node.has_value() || !node->has_parent())return VSXML$(node_t){n.ctx, nullptr};
    return make_node(n, node->parent());
}

VSXML$(node_t) VSXML$(node_first_child)(VSXML$(node_t) n){
    auto node = node_of(n);
    if(!node.has_value())return VSXML$(node_t){n.ctx, nullptr};
    for(auto& c : node->children()) return make_node(n, c);
    return VSXML$(node_t){n.ctx, nullptr};
}

VSXML$(node_t) VSXML$(node_next_sibling)(VSXML$(node_t) n){
    auto node = node_of(n);
    if(!node.has_value() || !node->has_next())return VSXML$(node_t){n.ctx, nullptr};
    return make_node(n, node->next());
}

size_t VSXML$(node_attr_count)(VSXML$(node_t) n){
    auto node = node_of(n);
    if(!node.has_value())return 0;
    auto range = node->attrs_range();
    if(!range.has_value())return 0;
    return (size_t)(range->second - range->first);
}

int VSXML$(node_attr_at)(VSXML$(node_t) n, size_t idx, VSXML$(str_t)* ns, VSXML$(str_t)* name, VSXML$(str_t)* value){
    auto node = node_of(n);
    if(!node.has_value())return (int)VSXML$(ErrorInvalidArgument);
    auto range = node->attrs_range();
    if(!range.has_value() || idx >= (size_t)(range->second - range->first))return (int)VSXML$(ErrorInvalidArgument);
    const auto* doc = static_cast<const VSXML$(document)*>(n.ctx);
    const attr_t* a = range->first + idx;
    if(ns != nullptr){ auto v = a->ns(); *ns = v.has_value() ? to_str(doc->raw().rsv(*v)) : VSXML$(str_t){nullptr, 0}; }
    if(name != nullptr){ auto v = a->name(); *name = v.has_value() ? to_str(doc->raw().rsv(*v)) : VSXML$(str_t){nullptr, 0}; }
    if(value != nullptr){ auto v = a->value(); *value = v.has_value() ? to_str(doc->raw().rsv(*v)) : VSXML$(str_t){nullptr, 0}; }
    return (int)VSXML$(Ok);
}

size_t VSXML$(node_text)(VSXML$(node_t) n, char* buf, size_t cap){
    auto node = node_of(n);
    if(!node.has_value())return 0;
    std::string out;
    for(char c : node->text()) out.push_back(c);
    if(buf != nullptr && cap > 0){
        const size_t k = out.size() < cap - 1 ? out.size() : cap - 1;
        std::memcpy(buf, out.data(), k);
        buf[k] = '\0';
    }
    return out.size();
}

VSXML$(query)* VSXML$(query_create)(VSXML$(error_t)* err){
    auto* out = new (std::nothrow) VSXML$(query);
    set_err(err, out != nullptr ? VSXML$(Ok) : VSXML$(ErrorAllocation));
    return out;
}
int VSXML$(query_accept)(VSXML$(query)* q){ if(q == nullptr)return (int)VSXML$(ErrorInvalidArgument); q->q.accept(); return (int)VSXML$(Ok); }
int VSXML$(query_child)(VSXML$(query)* q){ if(q == nullptr)return (int)VSXML$(ErrorInvalidArgument); q->q.child(); return (int)VSXML$(Ok); }
int VSXML$(query_descend)(VSXML$(query)* q){ if(q == nullptr)return (int)VSXML$(ErrorInvalidArgument); q->q.descend(); return (int)VSXML$(Ok); }
int VSXML$(query_fork)(VSXML$(query)* q){ if(q == nullptr)return (int)VSXML$(ErrorInvalidArgument); q->q.fork(); return (int)VSXML$(Ok); }
int VSXML$(query_match_type)(VSXML$(query)* q, int type){ if(q == nullptr)return (int)VSXML$(ErrorInvalidArgument); q->q.match_type((type_t)type); return (int)VSXML$(Ok); }
int VSXML$(query_match_name)(VSXML$(query)* q, const char* name){ if(q == nullptr)return (int)VSXML$(ErrorInvalidArgument); q->q.match_name(to_filter(name)); return (int)VSXML$(Ok); }
int VSXML$(query_match_ns)(VSXML$(query)* q, const char* ns){ if(q == nullptr)return (int)VSXML$(ErrorInvalidArgument); q->q.match_ns(to_filter(ns)); return (int)VSXML$(Ok); }
int VSXML$(query_match_value)(VSXML$(query)* q, const char* value){ if(q == nullptr)return (int)VSXML$(ErrorInvalidArgument); q->q.match_value(to_filter(value)); return (int)VSXML$(Ok); }
int VSXML$(query_match_text)(VSXML$(query)* q, const char* text){ if(q == nullptr)return (int)VSXML$(ErrorInvalidArgument); q->q.match_text(to_filter(text)); return (int)VSXML$(Ok); }
int VSXML$(query_match_attr)(VSXML$(query)* q, const char* ns, const char* name, const char* value){
    if(q == nullptr)return (int)VSXML$(ErrorInvalidArgument);
    q->q.match_attr(to_filter(ns), to_filter(name), to_filter(value));
    return (int)VSXML$(Ok);
}

int VSXML$(query_each)(const VSXML$(query)* q, VSXML$(node_t) root, VSXML$(query_cb) cb, void* user){
    if(q == nullptr || cb == nullptr)return (int)VSXML$(ErrorInvalidArgument);
    auto node = node_of(root);
    if(!node.has_value())return (int)VSXML$(ErrorInvalidArgument);
    q->q.for_each_while(*node, [&](wrp::base_t<unknown_t> n) -> bool {
        return cb(make_node(root, n), user) == 0;
    });
    return (int)VSXML$(Ok);
}

size_t VSXML$(query_count)(const VSXML$(query)* q, VSXML$(node_t) root){
    if(q == nullptr)return 0;
    auto node = node_of(root);
    if(!node.has_value())return 0;
    size_t count = 0;
    q->q.for_each(*node, [&](wrp::base_t<unknown_t>){ ++count; });
    return count;
}

void VSXML$(query_destroy)(VSXML$(query)* q){ delete q; }

} // extern "C"

#endif // VSXML_CXX_IMPL
