## What are queries?
Queries are the main mechanism to perform searches on the binary XML files. They can be used in three different clause types:

- `is` iterates over all nodes in a branch which satisfies the query criterion.
- `has` iterates over all nodes from a container (like a past query), for which the `is` clause is tested against the query criterion. Those for which a solution is defined will be picked.
- `maps` iterates over all nodes from a container (like a past query), and matches results for each subquery onto a map-like structure. 

## Structure of queries

A query is a linear sequence of query tokens, describing single filters for matching. Every query should end with an `accept()`.

### Basic commands

- `fork()` to force splitting matching by continuing here and expanding down.
- `accept()` to accept the current node and let the iterator go deeper.
- `type({...})` to match a subset of node type
- `attr({name, fn, ns})` if a given attribute (with namespace) satisfies the expressions (as string or boolean lambdas). 
- `match_ns({exp})` to match the namespace, with exp being either a string or a boolean lambda.
- `match_name({exp})` to match the name, with exp being either a string or a boolean lambda.
- `match_value({exp})` to match the value, with exp being either a string or a boolean lambda.
- `match_all_text({exp})` to match the text, with exp being either a string or a boolean lambda.

### String shorthands

Element-access operations are also provided in the form of simplified shorthands as strings:

- `"{ns}:{name}"` matching namespace and name. Special values `?` for each to determine anything matches. Empty string is considered empty string not match all.
- `*` to accept any node and move forward.
- `**` to recursively fork.

### Composition

- `operator*` to append right to the current (left) list
- `operator/` to append a `next()` AND right to left.
