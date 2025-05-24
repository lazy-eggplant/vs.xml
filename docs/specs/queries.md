## Structure of queries

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

- `"{ns}:{name}"` matching namespace and name. Special values `?` for each to determine anything matches. Empty string is considered empty string not match all.
- `*` to accept anything and move forward.
- `**` to recursively fork.

### Composition

- `operator*` to append right to the current list, or `operator/` to append a `next()` and the right filter.
