- [ ] Deprecate this file plz.
- [ ] Random access to attributes for the iterator.
- [ ] Tree builder method to use injection maps when generating the tree.

## Query redesign

The first implementation of queries was functional but also extremely bad to work with.  
It is plagued by issues with objects lifetime, coroutines in C++ are nice to use but quite bad in terms of memory footprint for this application.  
And we don't have properly serializable queries with simple hashing...  

So it is time to decide what was good about this first implementation and what to do so that the experience of the next is going to be better.  

- Design queries to work with the query builder, not as an afterthought but since the very beginning. The design work being done in [here](docs/specs/query-builder.md) must be extended.
- Drop co-routines. They were good to sketch a quick working solution, they are awful to harden against memory allocations and when handling issues related to their memory footprint.
- Keep the high level syntax the same, it is actually quite good I think.