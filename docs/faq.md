> Why the binary serialization of the XML is bigger compared to the base file?

That is to be expected. XML has very little overhead, with very little in terms of extra symbols and markers.  
By comparison, this library represents nodes by storing contextual information about their neighbours to speed up navigation.  
This additional pre-computed information explains the bigger memory footprint. Even when compressed via gzip, the source XML retains a meaningful edge.  

> So, why should I save the binary serialization?

Because you don't need to parse the original file again. You just need to memory-map it and use it as is.  
This lowers start-up time quite to zero if using `mmap`, and will only load pages for the content you need, regardless of the original file size.  
Such approach can massively boot real world performance on huge files, but also positively impact smaller ones as it operates lazily.

> What is the point of a representation which is not mutable? Why cannot I edit the tree?

Technically it is not immutable; however some types of mutations would be very expensive, almost like rebuilding the entire tree from scratch.  
Still, there are some operations which are quite fast, like reordering attributes in-place, changing strings with symbols already stored in memory, or adding annotations to the original document.  
Some of these operations are implemented by this library, others could be in a nearby future or delegated to downstream integrations.  

As for why, there is plenty of applications where mutability of the tree structure is only needed during construction, and only for the next token to append.  
This library tries to optimize such cases without introducing features which could compromise their performance.

> Why pointers are internally represented as relative to a base?

To allow them to be serialized, and still provide random access to the tree regardless of the device or medium on which it ends up being stored.  
This allows to share annotations of a tree for cheap, even while offloading or distributing computation.  
Also, results of complex queries are fully portable and easy to share.

> Why is XML serialization/de-serialization single threaded?

Simple answer, laziness. A more serious reply would be that serialization and de-serialization are meant to be just one-off operations.  
The bulk of whatever computation we must perform is going to act on the binary representation, not the original text.  
There is no trivial way to make XML parsing or serialization "parallel" without making the code involved much more complex.  
At which point it is generally better to waste threads for other tasks if possible.

> How to annotate a tree?

References to XML nodes and attributes have a method `addr` to return their portable address. Keeping a hash map or a similar data structure does the trick.  
You should be using a data structure which can be easily serialized/de-serialized, or even better which can be memory-mapped if feasible.  
The standard C++ library is not your best option, you might want to check for alternative like [gtl](https://github.com/greg7mdp/gtl).

> How to cache queries?

At the moment this is not directly supported by the library and will likely never be. However, we plan to introduce features to make it easier on downstream code.  
Right now, queries are not trivially hash-able. At some point we will add a query builder to structure query trees into linear buffers, more or less like we do with XML.  
The evolving plans for this feature are reported in a [specifications document](./specs/query-builder.md).  
Until then, a similar solution has to be implemented fully downstream.

> How to serialize/deserialize attributes of text fields into a different data type, like integers or floating point numbers?

XML has no intrinsic concept of "serialized data". All attributes and text values are string_views.  
We have schema languages built on top of XML, like RelaxNG. They usually allow annotating types for fields, but it has nothing to do with XML; hence, this library will not handle that.  
However, there are some intended workarounds:
- The symbol table can pretty much fit whatever the user's want. String views are not null terminated.  
  So, building a tree with `.raw_strings=true` would allow for arbitrary data, as long as no attempt is made to serialize the tree to XML from its binary representation.
- It is possible to represent the de-serialized values as annotations over tree nodes, and store them into a map-like container.  
  Those could either be saved alongside the original file (good if the serialization/de-serialization process is complex), or just generated when needed just to be cached for a faster retrieval.