## Breaking changes

- The `close` function for Tree(Raw) and Document(Raw) changed signature to return the new `Stored<T>` objects, so that storage is separate from the base classes to better use code in embedded and offloaded contexts. 
- Other minor differences might be present due to the changes discussed prior, it is too hard to spot and report all of them.
- Fixed signatures for `from_binary` removing string_views in place of `std::span<const uint8_t>`, and fixed some return types which were not set to `const`.