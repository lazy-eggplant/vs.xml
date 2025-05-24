Release to propagate (the minor) breaking changes downstream.  
We don't have many new features visible, but a lot of work in the background to improve the code base and its scope.  
For reference, `v0.2.x` should provide as main features: 

- Queries
- Multiple documents sharing symbols (and related binary format)
- Helpers for annotations (but they might be rescheduled for `v0.3.x` based on how much feature creep is met as it relates to tree injection which is its own beast to handle)

As minor features:

- Support for iterators to visit text across `text` and `cdata` boundaries.
- Apply replacement for serialization functions which don't use memory allocations in the serialization code (already implemented, not used)

Several steps have been made to implement both, but they cannot be tested still (probably there will be queries mostly done in v0.2.3).

### Changes

- Removed virtually all explicit exceptions.
- Removed memory allocations for a wide range of operations.
- Improvements to the binary format.
- Initial work to support shared symbol tables, annotations and queries. 
- More entry-points for customization, like some containers or predefined reduced memory layouts.
- Structural improvements in the repo, code quality etc.

### Breaking features

- The binary format changed.
- Several functions had some minor changes, like `[[nodiscard]]` being added, or `std::expected` in place of exceptions.