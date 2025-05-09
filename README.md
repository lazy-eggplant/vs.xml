> [!WARNING]  
> Ongoing project, the base functionality is ready, but more documentation and some advanced features have not been implemented yet.


This library offers a mostly-compliant[^1] XML parser, tree builder and several related utilities.  
It is not intended as a general purpose library, which means it might not be a good fit for your project.  
Please, read the rest of this readme to know more about its objectives and drawbacks.

### Features

- Support for a schema-less tree structure which can be fully relocated in disk, memory or offloaded devices without impacting its binary representation.
- Linked to the previous point, pointers/iterators based on this tree structure are random access, no need to navigate the tree to reach them.
- Good memory locality of the tree representation, making many operations on sub-trees trivial `memcpy`.
- Configurable memory footprint, the internal representation can decrease size for most of its fields properly run on "lesser" systems or improve cache performance.
- XML serialization and de-serialization.
- Naive support for namespaces[^2].

Non objectives:

- Support for arbitrary editing operations. As pointed out before, this library is special-purpose.  
  It is better for you to assume trees are immutable. They are not, and some interesting operations are allowed, but the memory layout dictates heavy restrictions to keep good performance on the rest.
- Extended XML entities, base64, DTD... none of that is needed for the intended target of this library.

### Supported platforms

This library is mostly standalone, but it requires support for the C standard library and a modern version of the C++ standard library.  
As such, it might not be the best option for embedded systems, but the current codebase could be adapted to be.  
Other dependencies are only used for the test-suite and benchmarks, they are not needed to build and install `vs.xml` to your system.

Also, a modern version of C++ should be used.  
I am working with C++23 for development, and I don't really plan on directly supporting older revisions of the language at the expense of code simplicity.  

[^1]: XML 1.0 is covered as a best-effort, but there will be small things where either the official XML standard or this implementation is going to be incompatible or a superset.  
      For more information on compatibility, please check [here](./docs/features.md).
[^2]: Namespaces are supported in the sense that the namespace is split from the element or attribute name if present, but its handling, validation or whatever is left to the user.


### External dependencies

While they are not used by the library itself, tests and benchmarks do:
- [mio](https://github.com/StrikerX3/mio) a simple way to handle memory mapped files.
- [nanobench](https://github.com/martinus/nanobench) to perform benchmarks.

### Licence

This library is released as `LGPL3.0`.  
Documentation is CC4.0 Attribution Share-Alike.  
Examples are CC0 unless something else is specified.