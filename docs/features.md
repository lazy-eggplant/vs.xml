## Standard compatibility

### XML parsing
- [x] namespaces [^1]
- [x] cdata
    - [ ] optional reduction to `text`
- [x] text
    - [ ] optional merging of close text blocks
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
    - [x] in tree/documents

[^1]: Partially, no validation for them