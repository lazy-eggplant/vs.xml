## Structure of queries

### Basic commands

- `next()` to force moving to the next layer in the filters.
- `capture()` to match the current node and let the iterator go deeper.
- `type<T>()` to match a node type
- `attr(name, fn)` if a given attribute satisfies `fn`, with specialized version where `fn` is just a string. 
- `attr(ns, name, fn)` if a given attribute (with namespace) satisfies `fn`, with specialized version where `fn` is just a string. 
- `text(fn)` if the text of the current node satisfies `fn`, with specialized version where `fn` is just a string. 
- `match_ns(fn)` with specialized version where `fn` is just a string.
- `match_name(fn)` with specialized version where `fn` is just a string.

### String shorthands

- `"{ns}:{name}"` equivalent to `type<element>() + match_ns(ns) + match_name(name) + capture() + next()`
- `"{ns}:*"` equivalent to `type<element>() + match_ns(ns) + capture() + next()`
- `"{name}"` equivalent to `type<element>() + match_ns("") + match_name(name) + capture() + next()`
- `*` equivalent to `capture() + next()`
- `?` equivalent to `capture() + next()` OR `next()`.
- `**` equivalent to `capture() + next()` OR `capture()`
- `*?` equivalent to `capture() + next()` OR `capture()` OR `next()`.

### Composition

- `operator+` or `operator/` to append the right filter to the current list.
