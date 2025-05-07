## Standard compatibility

## Features

### XML parsing
- [ ] namespaces [^1]
- [ ] cdata
    - [ ] optional reduction to `text`
- [ ] text
    - [ ] optional merging of close text blocks
- [ ] comments 
    - [ ] optional stripping
- [ ] basic entities

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

[^1]: Partially, no validation for them