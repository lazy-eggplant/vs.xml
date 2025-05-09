> [!WARNING]  
> Ongoing project, the base functionality is ready, but more documentation and some advanced features have not been implemented yet.


This library offers a mostly-compliant[^1] XML parser, tree builder and several related utilities.  
It is not intended as a general purpose library, which means it might not be a good fit for your project. Please, read the rest of this readme for more information.  

### Features

- Support for data structures which can be fully relocated, be it on disk, memory, offloaded devices etc.
- Somewhat related, support for pointers/iterators which can also be relocated and used without the need of navigating the tree or resolving a path.
- Good memory locality of the tree representation.
- Tunable memory footprint, the internal representation can decrease size of most fields to properly run on "lesser" systems.
- Naive support for namespaces[^2].

Non objectives:

- Support for arbitrary editing operations. As pointed out before, this library is special-purpose.  
  It is not going to compromise performance for the rest of the supported operations just to gain new capabilities in this sense.
- Extended XML entities, base64, DTD... none of that is needed for the intended target of this library.

### Supported platforms

This library is mostly standalone, but requires `libc` and a modern version of `libc++` or compatible alternatives.  
As such it is not really tailored for embedded systems, but it might be possible to derive a compatible version.

Also, a modern version of C++ should be used.  
I am working with C++23 for development, and I don't really plan on directly supporting older revisions of the language.  

[^1]: XML 1.0 is covered as a best-effort, but there will be small things where either the official XML standard or this implementation is going to be incompatible or a superset.  
      For more information on compatibility, please check [here](./docs/compatibility.md).
[^2]: Namespaces are supported in the sense that the namespace is split from the element or attribute name if present, but its handling, validation or whatever is left to the user.