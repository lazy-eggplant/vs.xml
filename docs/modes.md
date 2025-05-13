The `TreeBuilder` is parametrized by a configuration structure and initialized in ways which defines its behaviour and ownership of data.

### Configuration fields

- `allow_comments` if true allows comments to be appended in the tree, else they are silently skipped (but can still throw exceptions for validation)
- `allow_procs` if true allows processing directives to be appended in the tree, else they are silently skipped (but can still throw exceptions for validation)
- ...
- `raw_strings` if true, strings are assumed to be kept as is when serialized or deserialized from XML. Make sure you manually escape those used in comparisons or to append further data.
