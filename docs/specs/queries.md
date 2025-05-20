## Structure of queries

### Basic commands

- `next()` to force moving to the next layer in the filters.
- `accept()` to accept the current node and let the iterator go deeper.
- `type<T>()` to match a node type
- `attr(name, fn)` if a given attribute satisfies `fn`, with specialized version where `fn` is just a string. 
- `attr(ns, name, fn)` if a given attribute (with namespace) satisfies `fn`, with specialized version where `fn` is just a string. 
- `text(fn)` if the text of the current node satisfies `fn`, with specialized version where `fn` is just a string. 
- `match_ns(fn)` with specialized version where `fn` is just a string.
- `match_name(fn)` with specialized version where `fn` is just a string.

### String shorthands

- `"{ns}:{name}"` equivalent to `type<element>() + match_ns(ns) + match_name(name) + accept() + next()`
- `"{ns}:*"` equivalent to `type<element>() + match_ns(ns) + accept() + next()`
- `"{name}"` equivalent to `type<element>() + match_ns("") + match_name(name) + accept() + next()`
- `*` equivalent to `accept() + next()`
- `?` equivalent to `accept() + next()` OR `next()`.
- `**` equivalent to `accept() + next()` OR `accept()`
- `*?` equivalent to `accept() + next()` OR `accept()` OR `next()`.

### Composition

- `operator+` or `operator/` to append the right filter to the current list.
