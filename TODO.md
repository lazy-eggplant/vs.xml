- [ ] Add support for `&#...` escape sequences in xml serialization
- [x] Better ownership handling for the xml parser of the string_view/string
- [x] saving functions of trees without needing XML serialization.
- [x] loading functions of trees without needing XML serialization.
- [ ] Random access to attributes for the iterator.
- [ ] Support for modern C++ ranges.
- [ ] Tree builder method to use injection maps when generating the tree.
- [x] ~~Fix xml parsing bug when the file starts with `<?xml?>`~~ actually worse, we need to support a proper document which is embedding the tree
- [ ] ~~Enable a collapse mode for the builder where DATA/CDATA after a DATA/CDATA is collapsed.~~ as an alternative, a multi span iterator over text.

With these features, the library would be considered complete.