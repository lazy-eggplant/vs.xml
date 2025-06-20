## What are queries?
Queries are the main mechanism to perform search operations on the binary XML files.  
There are three main clause types:

- `any-is` iterates over all nodes form a container (like a prior query) as long as all criteria are met.
- `this-is` checks if a specific node satisfies requirements recursively navigating the parents list. In some sense it is a local version of `any-is` since any element satisfying `any-is` will satisfy `this-is` and the converse is true as well.
- `any-has` iterates over all nodes from a container (like a prior query), for which the `is` clause is tested against the query criterion. Those for which at least one solution is defined are picked.

All queries are designed to be capturing. For each token, you can pass two arguments:
- A label pattern
- A function (lambda) to register such pattern
This allows to record a map of captures for each valid match of the query.

## Structure of queries

A query is a linear sequence of query tokens. Most are meant to match specific features of the XML nodes, while some are used to control the "machine" running the validation process.  
Every complete query must end with an `accept()` token.

### Basic commands

- `fork()` to force splitting matching by continuing here and expanding down.
- `accept()` to accept the current node and let the iterator go deeper.
- `type({...})` to match a subset of node type.
- `match_ns({exp})` to match the namespace, with exp being either a string or a boolean lambda.
- `match_name({exp})` to match the name, with exp being either a string or a boolean lambda.
- `match_value({exp})` to match the value, with exp being either a string or a boolean lambda.
- `match_all_text({exp})` to match the text, with exp being either a string or a boolean lambda.
- `attr({name, fn, ns})` if a given attribute (with namespace) satisfies the expressions (as string or boolean lambdas). 

### About attributes

Attributes are handled differently compared to the rest of the XML nodes, because they are not. `attr` even in `is` queries will behave like a `has`. The presence of the attribute is being tested, but it will not be considered as the node to return. Still, they can be used as captures for a query.  
If you must match and return attributes directly, you will have to do that by manually iterating or filtering them, like `item.attrs() | std::views::filter(...)`.

### String shorthands

Several of the basic commands introduce earlier also have a simplified form based off string views:

- `"{ns}:{name}"` matching namespace and name. Special values `?` for each to determine anything matches that slot. Empty string is considered empty string, and not a generic "match all".
- `*` to accept any node and move forward.
- `**` to recursively fork.

### Composition

Tokens of a query can be composed by any of the following operators:

- `operator*` to append right to the current (left) list
- `operator/` to append a `next()` AND right to left.

## Applying queries

Queries are applied either by:

- Using the `is`, `has` or ~~`check`~~ functions.
- The operators `&`, `|` or ~~`==`~~ which are their respective alias.

Applied queries return asynchronous generators, so they can be further piped by `std::views::filter`.