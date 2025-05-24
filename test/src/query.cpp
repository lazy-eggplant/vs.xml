#include <vs-xml/query.hpp>
#include <vs-xml/builder.hpp>

#include <initializer_list>
#include <iostream>
#include <vector>
#include <coroutine>
#include <cstdlib>

#include <new> 
#include <print> 


// A very simple memory pool for demonstration purposes. DON'T SHIP IN THE LIBRARY!
struct MemoryPool {
    static constexpr size_t POOL_SIZE = 1024 * 10;
    char pool[POOL_SIZE];
    size_t offset = 0;

    size_t counter;

    void* allocate(size_t size) {
        std::print("new : {} {} {}\n", offset, size, counter);
        if (offset + size > POOL_SIZE) {
            return nullptr;
        }
        void* ptr = pool + offset;
        offset += size;
        counter++;
        return ptr;
    }

    void deallocate(){
        counter--;
    }
};

thread_local MemoryPool globalPool;

// Minimal Generator implementation.
template<typename T>
struct Generator {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type {
        T current_value;

        // Overload operator new to allocate from our custom memory pool.
        static void* operator new(std::size_t size) noexcept{
            if (void* ptr = globalPool.allocate(size)) {
                return ptr;
            }
            // Fall back to global new if pool is exhausted.
            //return ::operator new(size);
            return nullptr;
        }

        // Matching delete operator.
        static void operator delete(void* ptr, std::size_t size) {
            globalPool.deallocate();
            // In this demo we are not reclaiming memory from the pool.
            //::operator delete(ptr);
            return;
        }


        static auto get_return_object_on_allocation_failure() { return Generator{nullptr}; }
        auto get_return_object() { return Generator{handle_type::from_promise(*this)}; }
        auto initial_suspend() { return std::suspend_always{}; }
        auto final_suspend() noexcept { return std::suspend_always{}; }
        void unhandled_exception() { std::exit(1); }
        auto yield_value(T value) {
            current_value = value;
            return std::suspend_always{};
        }
        void return_void() {}
    };

    handle_type coro;

    explicit Generator(handle_type h) : coro(h) {}
    Generator(const Generator&) = delete;
    Generator(Generator&& rhs) : coro(rhs.coro) { rhs.coro = nullptr; }
    ~Generator() { if(coro) coro.destroy(); }

    // Iterator support for range-based for loops.
    struct iterator {
        handle_type coro;
        bool done;

        iterator(handle_type h, bool d) : coro(h), done(d) {}

        iterator& operator++() {
            coro.resume();
            done = coro.done();
            return *this;
        }
        const T& operator*() const { return coro.promise().current_value; }
        bool operator!=(const iterator& other) const { return done != other.done; }
    };

    iterator begin() {
        coro.resume();
        return {coro, coro.done()};
    }
    iterator end() { return {coro, true}; }
};

struct Node {
    int value;
    std::vector<Node> children;
    
    const std::vector<Node>& getChildren() const { return children; }
};

//TODO: instead of a single filter, apply one from a constexpr vector for each nesting, and stop when they are done.

#include <generator>

// A coroutine-based generator to recursively traverse the tree.
std::generator<const Node*> traverse(const Node& node, auto filter, int v) {

    if (filter(node)) {
        co_yield &node;
    }
    for (const auto& child : node.getChildren()) {
        // Recursively yield from the child generator.
        for (const Node* n : traverse(child, filter, v+1)) {
            co_yield n;
        }
    }
}

struct {
  mutable std::array<uint8_t,255> sliceA;
} constexpr reserved{};

template<std::array<uint8_t,255>::iterator BEGIN,std::array<uint8_t,255>::iterator END>
struct v{
    v(){
        for(auto it=BEGIN;it!=END;it++){
            *it = 44;
        }
        for(auto it=BEGIN;it!=END;it++){
            std::print("{} ",(int)*it);
        }
    }
};



template<xml::builder_config_t cfg>
auto mk_tree(){
    xml::TreeBuilder<cfg> build;
    build.reserve(100000,100000);
    build.begin("hello");
        build.x("AAA",{{"N1","N2"},{"N1","N3"}},[&]{
            build.x("BBB");
            build.comment("ss");
            build.comment("comment2");
        });
        build.x("a","AAA",{{"N1","N2"},{"N1","N3"}},[](auto& w) static{
            w.x("BBB");
            w.comment("ss");
            w.comment("comment2");
        });
        build.attr("op3-a", "v'>&al1");
        build.attr("op1-a-s", "val1", "w");
        build.attr("op2-a", "va&gt;l\"1");
        build.attr("op5-a", "va>l\"1");
        build.attr("op6-a-s", "val1", "w");
        build.comment("01234567890123456789Banana <hello ciao=\"worldo\" &amp; &></world>"); 
        build.begin("hello1","s");
        build.end();
        build.begin("hello2","s");
            build.text("Banana <hello ciao=\"worldo\" &amp; &></world>"); 
        build.end();
        build.begin("AAA","s");
            build.comment("hello");
            build.begin("hello5","s");
                build.x("BBB", {{"ATTR-0","TRUE"}});
            build.attr("op3", "val1");
            build.attr("op2", "val11");
            build.attr("op1", "val1");
            build.cdata("Hello'''''&&&& world!");
            build.end();
        build.end();
        build.begin("hello4","s");
        
        build.end();
    build.end();

    return build.close();
}

int main() {
    auto q = xml::query::query_t{}/xml::query::match_name({"hello"})/xml::query::accept()/xml::query::accept()/xml::query::next();
    constexpr auto q2 = xml::query::query_t<10>{}*xml::query::accept()*xml::query::accept();

    auto query_a = xml::query::query_t{}*"hello"/"**"/"BBB"*xml::query::match_attr({"ATTR-0"})*xml::query::accept();

    auto tree = *mk_tree<{.symbols=xml::builder_config_t::OWNED, .raw_strings=true}>();

    /*
    for(auto t : q.tokens){
        std::print("{}\n",t.args.index());
    }
    */
    for(const auto& t : xml::query::is(tree.root(),query_a)){
        std::print("{}\n", t.name().value_or("---"));
    }

/*
    v<reserved.sliceA.begin(),reserved.sliceA.end()> A;

    Node root{0, {
        {1, { {3, {}}, {4, {}} }},
        {2, { {5, {}}, {6, {}} }},
        {4, { {5, {}}, {6, {}} }}
    }};

    auto filterOdd = [](const Node& n) static { return (n.value % 2) != 0; };

    std::cout << "Lazy traversal with generators: ";
    for (const Node* n : traverse(root, filterOdd,0)) {
        std::cout << n->value << " ";
    }
    std::cout << std::endl;

*/

    return 0;
}