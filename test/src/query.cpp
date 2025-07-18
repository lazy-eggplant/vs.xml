#include <algorithm>
#include <vs-xml/query.hpp>
#include <vs-xml/tree-builder.hpp>
#include <vs-xml/filters.hpp>

#include <cstdlib>

#include <print> 

/*

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

*/


template<xml::builder_config_t cfg>
auto mk_tree(){
    xml::TreeBuilder<cfg> build;
    build.reserve({100000,100000});
    build.begin("root");
        build.x("node-a",{{"attr0","val0"},{"attr1","val1"}});
        build.x("node-b",{{"attr0","val0"},{"attr1","val1"}});
        build.x("node-a",{{"attr0","val0"},{"attr1","val1"}},[](auto& w) static{
            w.x("node-a",{{"attr0","val0"},{"attr1","val1"}});
        });

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

template<typename Iterator, typename Sentinel>
auto my_distance(Iterator first, Sentinel last) {
    decltype(first) temp = first;
    std::size_t n = 0;
    while (!(temp == last)) {
        temp++;
        ++n;
    }
    return n;
}
using namespace xml::query;


int main() {

    auto tree = *mk_tree<{.symbols=xml::builder_config_t::OWNED, .raw_strings=true}>();

    {
        auto value = *(tree.root() & query_t<0>{}*accept()).begin();
        assert(value.name().value_or("")=="root");
    }

    {
        auto query0 = query_t<0>{}/"node-a"*match_attr({"attr0"})*accept();
        auto container = tree.root() & query0;
        assert(std::ranges::distance(container)==2);
    }

    {
        auto query0 = query_t<0>{}/"*"*match_attr({"attr0","val0"})*accept();
        auto container = tree.root() & query0;
        assert(std::ranges::distance(container)==3);
    }

    {
        auto query0 = query_t<0>{}/"**"*match_attr({"attr0","val0"})*accept();
        auto container = tree.root() & query0;
        assert(std::ranges::distance(container)==4);
    }

    //TODO: Add more tests

    //auto q = xml::query::query_t{}/xml::query::match_name({"root"})/xml::query::accept()/xml::query::accept()/xml::query::next();
    //constexpr auto q2 = xml::query::query_t<10>{}*xml::query::accept()*xml::query::accept();

    auto query_a = xml::query::query_t{}*"root"/"**"/"BBB"*xml::query::accept();
    auto query_b = xml::query::query_t{}*xml::query::match_attr({"ATTR-0"})*xml::query::accept();
    //auto re = tree.root() & query_a & query_b;

    /*
    for(const auto& t : tree.root() & query_a | query_b | std::views::filter([](auto n) {return true;})){
        //std::print("{}\n", t.name().value_or("---"));
    }
    */
    return 0;
}