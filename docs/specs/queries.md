## What are queries?
Queries are the main mechanism to perform search operations on the binary XML files.  
There are three main clause types:

- `is` iterates over all nodes form a container (like a prior query) as long as all criteria are met.
- `has` iterates over all nodes from a container (like a prior query), for which the `is` clause is tested against the query criterion. Those for which at least one solution is defined are picked.
- ~~`maps`~~ (not implemented yet) iterates over all nodes from a container (like a prior query), and matches results for node in the query tree onto a map-like structure. 

## Structure of queries

A query is a linear sequence of query tokens. Most are meant to match specific features of the XML nodes, while some are used to control the "machine" running the query validation process.  
Every complete query should end with an `accept()` token.

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

Attributes are handled differently compared to the rest of the XML nodes, because they are not. `attr` will exclusively test the attribute and not bind or capture it. This is true even when running `is` clauses.  
If you must capture attributes, you will have to do that by manually iterating or filtering them, like `item.attrs() | std::views::filter(...)`.

### String shorthands

Several of the basic commands introduce earlier are also available in a simplified form, represented by string views:

- `"{ns}:{name}"` matching namespace and name. Special values `?` for each to determine anything matches that slot. Empty string is considered empty string, and not a generic "match all".
- `*` to accept any node and move forward.
- `**` to recursively fork.

### Composition

Tokens of a query can be composed by any of the following operators:

- `operator*` to append right to the current (left) list
- `operator/` to append a `next()` AND right to left.

## Applying queries

Queries are applied either by:

- Using the `is`, `has` or ~~`capture`~~ functions.
- The operators `&` and `|` which are alias for `is` and `has` respectively.

Applied queries return asynchronous generators, so they can be further piped by `std::views::filter`.