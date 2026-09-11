# Milestones

## `v0.3.1`

- [x] Harden binary loading (bounds, sections, symbols, alignment).
- [x] Ownership fixes (`StorageFor` moves, `Stored` move-only).
- [x] Parser correctness (error propagation, end-tag matching, depth limit, whitespace, invalid names).
- [x] Serializer consolidation + pretty-printing.
- [x] Query redesign: explicit builder (`xml::query::query_t`).
- [x] C bindings (`vs-xml/c.h`).
- [x] xmake build and `vs-xml/` include layout.
- [ ] Documentation coverage.

## Query engine

- [x] Explicit builder API replacing the earlier generator.
- [x] Single canonical query API (the competing `QueryBuilder`/`query-new` drafts were removed).
- [ ] Serializable queries with simple hashing.
- [ ] Random access to attributes for the iterator.
- [ ] Capturing tree-queries.

## Later

- [ ] `validate_xml_label` performance / disable flag.
- [ ] Remove recursive calls in `Parser` and `print`.
- [ ] Remove the stack from `TreeBuilder`.
- [ ] Archive: order documents by name, and CLI compress/decompress.
- [ ] Replace the packed header / flexible array with a standard layout (next format major).
- [ ] Abstract `std` containers consistently via the `fwd/` aliases.
- [ ] Embedded-friendly and offloading trackers.
