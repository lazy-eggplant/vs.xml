- Add support for `&#...` escape sequences in xml serialization
- Better ownership handling for the xml parser of the string_view/string
- Support for saving/loading functions of trees without needing serialization.
- Random access to attributes for the iterator.
- Tree builder method to use injection maps when generating the tree.
- Fix xml parsing bug when the file starts with `<?xml?>`
- Enable a collapse mode for the builder where DATA/CDATA after a DATA/CDATA is collapsed.

With these features, the library would be considered complete.