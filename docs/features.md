## Standard compatibility

General compatibility with the XML 1.0 specs

### XML parsing
- [x] namespaces [^1]
- [x] cdata
- [x] text
- [x] comments (with optional stripping)
- [x] processing directives (with optional stripping)
- [x] basic entities
- [x] complex entities

### XML serialization
- [x] namespaces
- [x] processing
- [x] cdata
- [x] text
- [x] comments
- [ ] output prettifycation
    - [ ] indentation
    - [ ] newline style
    - [ ] other linting

## Library features

### Tree builder
- [x] namespaces
- [x] processing
- [x] cdata
- [x] text
- [x] comments

### Algorithms
- [x] iterators for nodes
- [x] iterators for attributes
- [x] Unified iterator for text of an element, scanning all text/CDATA children in a single element.
- [ ] tree/sub-tree cloning
    - [x] Basic copy
    - [ ] String compression
- [x] attributes reordering
- [ ] node injection
- [x] simplified tree wrapper to avoid xml::sv->string_view conversions.
- [x] documents
- [x] archives
### Queries

## Portability

Issues concerning usage in [embedded](embedded.md) and [offloaded](offloading.md) contexts. Mode details in the respective pages.

- [x] `noexcept` mode
- [x] `noassert` mode
- [ ] no memory allocations
    - [x] for the tree/document/archive binary representation
    - [ ] in the query builder[^2]
    - [ ] in tree building[^2]
    - [ ] in query processing[^3]

[^1]: Partially, no validation for them
[^2]: The objective is to pass custom allocators so that arena strategies can be implemented for example.
[^3]: Queries allocates as a stack, so specific strategies and allocator can be adopted for them.