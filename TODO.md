- [x] Add support for `&#...` escape sequences in xml serialization
- [x] Better ownership handling for the xml parser of the string_view/string
- [x] saving functions of trees without needing XML serialization.
- [x] loading functions of trees without needing XML serialization.
- [ ] Random access to attributes for the iterator.
- [x] Support for modern C++ ranges.
- [ ] Tree builder method to use injection maps when generating the tree.
- [x] ~~Fix xml parsing bug when the file starts with `<?xml?>`~~ actually worse, we need to support a proper document which is embedding the tree
- [x] ~~Enable a collapse mode for the builder where DATA/CDATA after a DATA/CDATA is collapsed.~~ as an alternative, a multi span iterator over text.
- [ ] ~~Align symbols to make access comparisons etc faster.~~ nah, lost memory is worst.

## Query redesign

The first implementation of queries was functional but also extremely bad to work with.  
It is plagued by issues with objects lifetime, coroutines in C++ are nice to use but quite bad in terms of memory footprint for this application.  
And we don't have properly serializable queries with simple hashing...  

So it is time to decide what was good about this first implementation and what to do so that the experience of the next is going to be better.  

- Design queries to work with the query builder, not as an afterthought but since the very beginning. The design work being done in [here](docs/specs/query-builder.md) must be extended.
- Drop co-routines. They were good to sketch a quick working solution, they are awful to harden against memory allocations and when handling issues related to their memory footprint.
- Keep the high level syntax the same, it is actually quite good I think.