This release is very dense release in terms of features!  
Most of them have been under development for a while, but they just about reached the "untested but useful" state:

- Queries on the XML tree are now working! With shorthands and everything. They need a good documentation page as they are one of the core features to properly use this library.
- (breaking) changes in the binary format to support multi-document archives sharing symbols. No capability exposed to the library yet, but the binary format should be "final".
- The configuration flags used while configuring the project are now shipped with the library headers.
- A new `text` iterator to access all `text` and `cdata` sections of an element in one go. It is kind of slow compared to direct access of the string-view. For some operations, filtering text/cdata children and iterate over their values will be a much better approach. Still, it can be good for queries or to implement searches.

## Future query redesign

This first implementation of queries was very informative and technically functional... but also very annoying to work with.  
It is plagued by issues linked to object lifetimes, which are not as easy to unravel due to the presence of co-routines in the mix.  
By the way, coroutines in C++ are nice to use but quite bad in terms of memory footprint for this application, and while mitigations are possible, memory is going to be higher compared to an iterator-based approach.  

Furthermore, we don't have properly serializable queries with simple hashing...  
So, for the next release, it will be time to decide what was good about this first implementation and what to do so that the experience of the next is going to be better.  

- Design queries to work with the query builder, not as an afterthought but since the very beginning. The design work being done in [here](docs/specs/query-builder.md) must be extended.
- Drop co-routines. They were good to sketch a quick working solution, they are awful to harden against memory allocations and when handling issues related to their memory footprint.
- Keep the high level syntax the same, it is actually quite good I think.
