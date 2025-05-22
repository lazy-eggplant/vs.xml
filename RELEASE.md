- Removed virtually all explicit exceptions.
- Removed memory allocations for a wide range of operations.
- Improvements to the binary format.
- Initial work to support shared symbol tables, annotations and queries. 
- More entry-points for customization, like some containers or predefined reduced memory layouts.
- Structural improvements in the repo, code quality etc.

Breaking features:
- The binary format changed.
- Several functions had some minor changes, like `[[nodiscard]]` being added, or `std::expected` in place of exceptions.