Builders are parametrized by a configuration structure and initialized in ways which defines its behaviour and ownership of data.

### Configuration fields

- `allow_comments` if true allows comments to be appended in the tree, else they are silently skipped (but can still throw exceptions for validation)
- `allow_procs` if true allows processing directives to be appended in the tree, else they are silently skipped (but can still throw exceptions for validation)
- `symbols`:
    - `EXTERN_ABS` using full memory in absolute position; the generated tree cannot be saved as binary.
    - `EXTERN_REL` can be saved as binary, but it requires the tree to bind a symbol table later when constructed.
    - `OWNED` no compression of symbols, but they are owned.
    - `COMPRESS_LABELS` only compressing symbols which are used for tag and prop labels (and namespaces).
    - `COMPRESS_ALL` all symbols are compressed.
- `raw_strings` if true, strings are assumed to be kept as is when serialized or deserialized from XML. Make sure you manually escape those used in comparisons or to append further data.
