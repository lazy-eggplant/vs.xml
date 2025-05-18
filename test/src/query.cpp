#include <iostream>
#include <vector>
#include <coroutine>
#include <cstdlib>

// Minimal Generator implementation.
template<typename T>
struct Generator {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type {
        T current_value;
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

// A coroutine-based generator to recursively traverse the tree.
Generator<const Node*> traverse(const Node& node, auto filter) {
    if (filter(node)) {
        co_yield &node;
    }
    for (const auto& child : node.getChildren()) {
        // Recursively yield from the child generator.
        for (const Node* n : traverse(child, filter)) {
            co_yield n;
        }
    }
}

int main() {
    Node root{0, {
        {1, { {3, {}}, {4, {}} }},
        {2, { {5, {}}, {6, {}} }}
    }};

    auto filterOdd = [](const Node& n) { return (n.value % 2) != 0; };

    std::cout << "Lazy traversal with generators: ";
    for (const Node* n : traverse(root, filterOdd)) {
        std::cout << n->value << " ";
    }
    std::cout << std::endl;

    return 0;
}