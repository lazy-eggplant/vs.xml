# Queries

Queries search a subtree of a document. They are built with an explicit builder
and evaluated iteratively, so the memory footprint is
predictable and bounded by the tree depth.

## Building

Start from `xml::query::query_t` and append steps:

- `accept()`: emit the current node and stop the branch.
- `child()`: move to the children of the current element.
- `descend()`: move to every descendant of the current element.
- `fork()`: stay on the current node and every descendant (descendant-or-self).
- `match_type(type_t)`: keep only nodes of a given type.
- `match_ns(filter)`, `match_name(filter)`, `match_value(filter)`, `match_text(filter)`:
  keep only nodes whose namespace, name, value, or direct text matches.
- `match_attr(ns, name, value)`: keep elements carrying an attribute that matches
  all three filters.

A `filter_t` is a wildcard (`query::any()`), an exact string (`query::eq("x")` or
a bare `const char*`), or a predicate stored as
`std::move_only_function<bool(std::string_view) const>`.

A branch that reaches the end of the steps is emitted implicitly; `accept()` is
only needed to stop a branch early or to accept an intermediate step.

Because predicates are move-only, `query_t` is move-only: construct a named
query and append steps to it.

## Evaluating

- `for_each(node, fn)`: call `fn` for every accepted node.
- `for_each_while(node, fn)`: like `for_each`, but stop when `fn` returns false.
- `collect(node)`: return a vector of accepted nodes, in document order.
- `has(node)`: true if at least one node is accepted.

## Example

```cpp
xml::query::query_t q;
q.child().element("item")
 .match_attr(query::any(), query::eq("id"), query::any())
 .accept();

for(auto node : q.collect(document.root())){
    // ... use node ...
}
```

## C bindings

The same builder is exposed through `vsxml_query_*` in `<vs-xml/c.h>`. See
`test/c-interface.c` for a complete example.
