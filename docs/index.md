> [!WARNING]  
> Ongoing project. Many features are still missing and documentation has not a full coverage.


This library offers a mostly-compliant[^1] XML parser, tree builder, query engine and several related utilities.  
It is not intended as a general purpose library, which means it might not be a good fit for your project.  
Please, read the rest of the [original readme](https://github.com/lazy-eggplant/vs.xml) to know more about its objectives and drawbacks.  
Or continue with this documentation which offers a more in-depth description of several topics.

## Features

- Support for a schema-less tree structure which can be fully relocated in disk, memory or offloaded devices without impacting its binary representation.
- Linked to the previous point, pointers/iterators based on this tree structure are random access, no need to navigate the tree to reach them.
- Good memory locality of the tree representation, making many operations on sub-trees trivial `memcpy`.
- Configurable memory footprint, the internal representation can decrease size for most of its fields properly run on "lesser" systems or improve cache performance.
- An efficient engine to perform queries on a document, all based on lazy evaluation.
- XML serialization and de-serialization.
- Naive support for namespaces[^2].

Non objectives:

- Support for arbitrary editing operations. This library is special-purpose, so only a small number of mutable operations will be supported to keep the rest as fast as possible.
- Extended XML entities, base64, DTD... none of that is needed for the intended target of this library.
- In general, being fully XML compliant.


[^1]: XML 1.0 is covered as a best-effort, but there will be small things where either the official XML standard or this implementation is going to be incompatible or a superset.  
      For more information on compatibility, please check [here](./features.md).

[^2]: Namespaces are supported in the sense that the namespace is split from the element or attribute name if present, but its handling, validation or whatever is left to the user.
