## Standard compatibility

### XML parsing
- [x] namespaces [^1]
- [x] cdata
- [x] text
    - [x] Unified iterator for text of an element, scanning all text/CDATA children.
- [x] comments 
    - [x] optional stripping
- [x] basic entities
- [x] complex entities

### XML serialization
- [x] namespaces
- [x] processing
- [x] cdata
- [x] text
- [x] comments
- [ ] output configurability
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
- [ ] tree/sub-tree cloning
    - [x] Basic copy
    - [ ] String compression
- [x] attributes reordering
- [ ] node injection
- [x] simplified tree wrapper to avoid xml::sv->string_view conversions.

### Queries

## Portability

- [x] `noexcept` mode
- [x] `noassert` mode
- [ ] no memory allocations
    - [x] for the tree/documents binary representation
    - [ ] in the query builder[^2]
    - [ ] in tree building[^2]
    - [ ] in query processing[^3]

[^1]: Partially, no validation for them
[^2]: The objective is to pass custom allocators so that arena strategies can be implemented for example.
[^3]: Queries allocates as a stack, so specific strategies and allocator can be adopted for them.