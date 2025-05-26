#pragma once

/**
 * @file query.hpp
 * @author karurochari
 * @brief Queries on a tree.
 * @date 2025-05-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */

//Temporary add custom implementation here later
#include <cstddef>
#include <generator>
#include <functional>
#include <string_view>
#include <variant>
#include <vector>
#include <vs-xml/commons.hpp>
#include <vs-xml/private/wrp-impl.hpp>

namespace VS_XML_NS{
namespace query{

template<size_t N = 0>
struct query_t;

//TODO: Hide it somewhere 
//TODO: after it is verified working, move to the custom allocator plz.

/**
 * @brief generator-like implementation for query results.
 * @details similar to std::generator, but using a custom allocation scheme
 */
template<typename T>
class generator {
public:
    struct promise_type;

    using handle_type = std::coroutine_handle<promise_type>;
    using internal_type = T;

    // The promise type required by the coroutine.
    struct promise_type {
        uint8_t buffer[sizeof(internal_type)];
        std::exception_ptr exception;

        // Overload operator new to allocate from our custom memory pool.
        static void* operator new(std::size_t size) noexcept{
            //if (void* ptr = globalPool.allocate(size)) {
            //    return ptr;
            //}
            // Fall back to global new if pool is exhausted.
            return ::operator new(size);
            //return nullptr;
        }

        // Matching delete operator.
        static void operator delete(void* ptr, std::size_t size) {
            //globalPool.deallocate();
            // In this demo we are not reclaiming memory from the pool.
            ::operator delete(ptr);
            //return;
        }
        
        static auto get_return_object_on_allocation_failure() { return generator{nullptr}; }
        auto get_return_object() { return generator{handle_type::from_promise(*this)}; }
        auto initial_suspend() { return std::suspend_always{}; }
        auto final_suspend() noexcept { return std::suspend_always{}; }
        void unhandled_exception() { exception = std::current_exception(); /*std::exit(1);*/ }
        auto yield_value(internal_type value) {
            new(buffer) internal_type(value);
            //current_value = value;
            return std::suspend_always{};
        }
        void return_void() {}
    };

    // Iterator class to make the generator conform to the range concept.
    class iterator {
    public:
        using coro_handle = std::coroutine_handle<promise_type>;
        using iterator_category = std::input_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;

        iterator() noexcept : handle(nullptr) {}

        explicit iterator(coro_handle h) : handle(h) {
            // Preload the first value: resume the coroutine.
            move_to_next();
        }

        iterator& operator++() {
            move_to_next();
            return *this;
        }
        // Postfix ++
        iterator operator++(int) {auto t = *this; move_to_next(); return t;}

        const T& operator*() const {
            return *(T*)handle.promise().buffer;
        }
        const T* operator->() const {
            return std::addressof(operator*());
        }

        bool operator==(std::default_sentinel_t) const {
            return !handle || handle.done();
        }

    private:
        void move_to_next() {
            if (handle) {
                handle.resume();
                if (handle.promise().exception) {
                    std::rethrow_exception(handle.promise().exception);
                }
            }
        }

        coro_handle handle;
    };

    using iterator_t = iterator;

    generator() noexcept : handle(nullptr) {}
    explicit generator(std::coroutine_handle<promise_type> h) : handle(h) {}

    // Disable copying but allow moving.
    generator(const generator&) = delete;
    generator(generator&& other) noexcept : handle(other.handle) {
        other.handle = nullptr;
    }
    generator& operator=(const generator&) = delete;
    generator& operator=(generator&& other) noexcept {
        if (this != &other) {
            if (handle)
                handle.destroy();
            handle = other.handle;
            other.handle = nullptr;
        }
        return *this;
    }
    
    ~generator() {
        if (handle)
            handle.destroy();
    }
    
    iterator_t begin() {
        return iterator_t{ handle };
    }
    std::default_sentinel_t end() {
        return {};
    }
    
private:
    std::coroutine_handle<promise_type> handle;
};

using result_t = generator<wrp::base_t<unknown_t>>;

struct token_t{
    typedef std::variant<std::monostate,std::string_view,std::function<bool(std::string_view)>>  operand_t;

    enum type_t : uint8_t{
        /*Empty*/
        ACCEPT, 
        NEXT, 
        FORK,
        /*type_filter*/
        TYPE,
        /*Unary sv*/
        MATCH_NS, MATCH_NAME, MATCH_VALUE, MATCH_ALL_TEXT,
        /*Attr*/
        MATCH_ATTR
    };
    
    template<type_t T>
    struct empty_t{};

    template<type_t T>
    struct single_t : operand_t{};

    template<type_t T>
    struct type_filter_t{
        uint8_t is_element:1 = false ;
        uint8_t is_comment:1 = false ;
        uint8_t is_proc:1 = false ;
        uint8_t is_text:1 = false ;
        uint8_t is_cdata:1 = false ;
        uint8_t is_marker:1 = false ;
    };

    template<type_t T>
    struct attr_t{
        operand_t name = std::monostate{};
        operand_t value = std::monostate{};
        operand_t ns = std::monostate{};
    };

    using args_t = 
        std::variant<
            empty_t<ACCEPT>,
            empty_t<NEXT>,
            empty_t<FORK>,
            type_filter_t<TYPE>,
            single_t<MATCH_NS>,
            single_t<MATCH_NAME>,
            single_t<MATCH_VALUE>,
            single_t<MATCH_ALL_TEXT>,
            attr_t<MATCH_ATTR>
        > ;

    args_t args;

    constexpr token_t(const args_t& t={}):args(t){}
};


constexpr static token_t accept() {
    return {token_t::empty_t<token_t::ACCEPT>{}};
}

constexpr static token_t fork() {
    return {token_t::empty_t<token_t::FORK>{}};
}

constexpr static token_t next() {
    return {token_t::empty_t<token_t::NEXT>{}};
}

constexpr static token_t type(token_t::type_filter_t<token_t::TYPE> arg) {
    return {arg};
}

constexpr static token_t match_ns(token_t::single_t<token_t::MATCH_NS> arg) {
    return {arg};
}

constexpr static token_t match_name(token_t::single_t<token_t::MATCH_NAME> arg) {
    return {arg};
}

constexpr static token_t match_value(token_t::single_t<token_t::MATCH_VALUE> arg) {
    return {arg};
}

constexpr static token_t match_all_text(token_t::single_t<token_t::MATCH_ALL_TEXT> arg) {
    return {arg};
}

constexpr static token_t match_attr(token_t::attr_t<token_t::MATCH_ATTR> arg) {
    return {arg};
}


extern std::pair<std::string_view, std::string_view> split_on_colon(std::string_view input);

template<size_t N>
struct query_t{
    using container_type = std::array<token_t,N>;

    container_type tokens;
    size_t current=0;

    constexpr query_t& operator * (std::string_view str){
        if(str=="*") return *this;
        else if(str=="**") return *this * fork();
        else{
            auto [ns,name] = split_on_colon(str);
            if(ns=="?" && name=="?") return *this * type({.is_element=true});
            else if (ns=="?") return *this * type({.is_element=true}) * match_name({name});
            else if (name=="?") return *this * type({.is_element=true}) * match_ns({ns});
            else return *this * type({.is_element=true}) * match_ns({ns}) * match_name({name});
        }
    }

    constexpr query_t& operator * (const token_t& tkn){tokens[current]=tkn;current++;return *this;}

    constexpr inline query_t& operator / (std::string_view str){
        return *this * next() * str;
    }

    constexpr inline query_t& operator / (const token_t& tkn){
        return *this * next() * tkn;
    }

};

template<>
struct query_t<0>{
    using container_type = std::vector<token_t>;

    container_type tokens;

    constexpr query_t& operator * (std::string_view str){
        if(str=="*") return *this;
        else if(str=="**") return *this * fork();
        else{
            auto [ns,name] = split_on_colon(str);
            if(ns=="?" && name=="?") return *this * type({.is_element=true});
            else if (ns=="?") return *this * type({.is_element=true}) * match_name({name});
            else if (name=="?") return *this * type({.is_element=true}) * match_ns({ns});
            else return *this * type({.is_element=true}) * match_ns({ns}) * match_name({name});
        }
    }

    constexpr query_t& operator * (const token_t& tkn){tokens.push_back(tkn);return *this;}

    constexpr inline query_t& operator / (std::string_view str){
        return *this * next() * str;
    }

    constexpr inline query_t& operator / (const token_t& tkn){
        return *this * next() * tkn;
    }

};

template<size_t N>
result_t is(wrp::base_t<unknown_t> root, typename query_t<N>::container_type::const_iterator begin, typename query_t<N>::container_type::const_iterator end);

template<size_t N=0>
inline result_t is(wrp::base_t<unknown_t> root, const query_t<N>& query) {
    return is<N>(root, query.tokens.begin(), query.tokens.end());
}

template<size_t N=0>
inline result_t is(result_t&& src , const query_t<N>& query) {
    for(auto element : src){
        return is(element, query.tokens.begin(), query.tokens.end());
    }
}

template<size_t N=0>
inline result_t operator&(wrp::base_t<unknown_t> src, const query_t<N>& query){return is(src,query);}

template<size_t N=0>
inline result_t operator&(result_t&& src, const query_t<N>& query){return is(std::move(src),query);}

//TODO: not tested

template<size_t N=0>
inline result_t has(wrp::base_t<unknown_t> root, const query_t<N>& query) {
    auto b = false;
    for(auto c : is<N>(root, query.tokens.begin(), query.tokens.end())){b=true;break;}
    if(b)co_yield root;
    co_return;
}

template<size_t N=0>
inline result_t has(result_t&& src, const query_t<N>& query) {
    for(auto element : src){
        //Does this stop after the first match is found, as that is sufficient? Should we also return those matches somehow?
        auto b = false;
        for(auto c : is<N>(element, query.tokens.begin(), query.tokens.end())){b=true;break;}
        if(b)co_yield element;
    }
    co_return;
}

template<size_t N=0>
inline result_t operator|(wrp::base_t<unknown_t> src, const query_t<N>& query){return has(src,query);}

template<size_t N=0>
inline result_t operator|(result_t&& src, const query_t<N>& query){return has(std::move(src),query);}

}
}

template<> 
inline constexpr bool std::ranges::enable_borrowed_range<VS_XML_NS::query::result_t> = true;


#include <vs-xml/private/query-impl.hpp>