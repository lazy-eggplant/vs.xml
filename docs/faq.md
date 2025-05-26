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