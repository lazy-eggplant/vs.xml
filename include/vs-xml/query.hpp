#pragma once

/**
 * @file query.hpp
 * @author karurochari
 * @brief Tree queries.
 * @date 2025-05-18
 *
 * @copyright Copyright (c) 2025
 *
 * @details
 * The query API is an explicit, handle-free builder: steps are appended with
 * plain method calls and evaluated iteratively, with a predictable and bounded
 * memory footprint.
 *
 * Steps, in order:
 *   - accept()   : emit the current node and stop the branch.
 *   - child()    : move to the children of the current element.
 *   - descend()  : move to every descendant of the current element.
 *   - fork()     : stay on the current node and every descendant.
 *   - match_type/match_ns/match_name/match_value/match_text/match_attr:
 *                  filter the current node; the branch is pruned on no match.
 *
 * A branch that reaches the end of the steps is emitted implicitly.
 */

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <vs-xml/commons.hpp>
#include <vs-xml/fwd/vector.hpp>
#include <vs-xml/wrp-node.hpp>

namespace VS_XML_NS{
namespace query{

using node_t = wrp::base_t<unknown_t>;

/**
 * @brief A single string filter: wildcard, exact match or predicate.
 */
struct filter_t{
    using predicate_t = std::move_only_function<bool(std::string_view) const>;

    std::variant<std::monostate, std::string_view, predicate_t> value;

    filter_t() = default;
    filter_t(std::string_view s):value(s){}
    filter_t(const char* s):value(std::string_view(s)){}
    explicit filter_t(predicate_t f):value(std::move(f)){}

    bool match(std::string_view got) const{
        if(std::holds_alternative<std::monostate>(value))return true;
        if(std::holds_alternative<std::string_view>(value))return std::get<std::string_view>(value)==got;
        return std::get<predicate_t>(value)(got);
    }
};

///Wildcard filter, matching any value.
inline filter_t any(){return filter_t{};}
///Exact-match filter.
inline filter_t eq(std::string_view s){return filter_t{s};}

/**
 * @brief A compiled path query.
 */
struct query_t{
    enum class op : uint8_t{
        ACCEPT, CHILD, DESCEND, FORK,
        TYPE, MATCH_NS, MATCH_NAME, MATCH_VALUE, MATCH_TEXT, MATCH_ATTR
    };

    struct step_t{
        op code;
        type_t type = type_t::UNKNOWN;
        filter_t ns{}, name{}, value{};
    };

    vector<step_t> steps;

    query_t& accept(){steps.push_back({op::ACCEPT});return *this;}
    query_t& child(){steps.push_back({op::CHILD});return *this;}
    query_t& descend(){steps.push_back({op::DESCEND});return *this;}
    query_t& fork(){steps.push_back({op::FORK});return *this;}

    query_t& match_type(type_t t){steps.push_back({op::TYPE,t});return *this;}
    query_t& match_ns(filter_t f){
        steps.push_back({op::MATCH_NS,type_t::UNKNOWN,std::move(f)});
        return *this;
    }
    query_t& match_name(filter_t f){
        steps.push_back({op::MATCH_NAME,type_t::UNKNOWN,{},std::move(f)});
        return *this;
    }
    query_t& match_value(filter_t f){
        steps.push_back({op::MATCH_VALUE,type_t::UNKNOWN,{},std::move(f)});
        return *this;
    }
    query_t& match_text(filter_t f){
        steps.push_back({op::MATCH_TEXT,type_t::UNKNOWN,{},std::move(f)});
        return *this;
    }
    query_t& match_attr(filter_t ns, filter_t name, filter_t value){
        steps.push_back({op::MATCH_ATTR,type_t::UNKNOWN,std::move(ns),std::move(name),std::move(value)});
        return *this;
    }

    ///Convenience for matching an element by local name, in any namespace.
    query_t& element(filter_t name = {}){
        match_type(type_t::ELEMENT);
        return match_name(std::move(name));
    }

    /**
     * @brief Run the query over a subtree, invoking `fn` for every accepted node.
     */
    template<typename Fn>
    void for_each(node_t root, Fn&& fn) const{
        auto f = [&](node_t n) -> bool { fn(n); return true; };
        run(root, 0, f);
    }

    /**
     * @brief Like for_each, but `fn` returns false to stop the traversal early.
     * @return true if the traversal completed, false if it was stopped.
     */
    template<typename Fn>
    bool for_each_while(node_t root, Fn&& fn) const{
        auto f = std::forward<Fn>(fn);
        return run(root, 0, f);
    }

    ///Collect all accepted nodes, in document order.
    [[nodiscard]] vector<node_t> collect(node_t root) const{
        vector<node_t> out;
        for_each(root,[&](node_t n){out.push_back(n);});
        return out;
    }

    ///True if at least one node is accepted.
    [[nodiscard]] bool has(node_t root) const{
        bool found = false;
        for_each_while(root,[&](node_t){found = true; return false;});
        return found;
    }

private:
    template<typename Fn>
    bool run(node_t node, size_t idx, Fn& fn) const{
        for(size_t i=idx;i<steps.size();i++){
            const step_t& s = steps[i];
            switch(s.code){
                case op::ACCEPT:
                    return fn(node);
                case op::CHILD:
                    if(node.type()==type_t::ELEMENT)
                        for(auto& c : node.children()) if(!run(c, i+1, fn))return false;
                    return true;
                case op::DESCEND:
                    if(node.type()==type_t::ELEMENT)
                        for(auto& c : node.children()) if(!subtree(c, true, i+1, fn))return false;
                    return true;
                case op::FORK:
                    if(!run(node, i+1, fn))return false;
                    if(node.type()==type_t::ELEMENT)
                        for(auto& c : node.children()) if(!subtree(c, true, i+1, fn))return false;
                    return true;
                case op::TYPE:
                    if(node.type()!=s.type)return true;
                    break;
                case op::MATCH_NS:
                    if(!optional_match(s.ns, node.ns()))return true;
                    break;
                case op::MATCH_NAME:
                    if(!optional_match(s.name, node.name()))return true;
                    break;
                case op::MATCH_VALUE:
                    if(!optional_match(s.value, node.value()))return true;
                    break;
                case op::MATCH_TEXT:
                    if(!s.value.match(node_text(node)))return true;
                    break;
                case op::MATCH_ATTR:
                    if(!match_attr_node(node, s))return true;
                    break;
            }
        }
        return fn(node); //Implicit accept at the end of the query.
    }

    template<typename Fn>
    bool subtree(node_t node, bool include, size_t idx, Fn& fn) const{
        if(include && !run(node, idx, fn))return false;
        if(node.type()==type_t::ELEMENT)
            for(auto& c : node.children()) if(!subtree(c, true, idx, fn))return false;
        return true;
    }

    template<typename Exp>
    static bool optional_match(const filter_t& f, const Exp& e){
        if(!e.has_value())return std::holds_alternative<std::monostate>(f.value);
        return f.match(std::string_view(*e));
    }

    static bool match_attr_node(node_t node, const step_t& s){
        if(node.type()!=type_t::ELEMENT)return false;
        for(auto& a : node.attrs()){
            if(optional_match(s.ns,a.ns()) && optional_match(s.name,a.name()) && optional_match(s.value,a.value()))
                return true;
        }
        return false;
    }

    static std::string node_text(node_t node){
        std::string out;
        for(char c : node.text()) out.push_back(c);
        return out;
    }
};

}
}
