> Why the binary serialization of the XML is bigger compared to the base file?

That is to be expected. XML has very little overhead, with extra symbols and marker.  
By comparison, nodes as represented by this library are storing contextual information about their neighbours to speed up navigation.  
This additional pre-computed information explains the bigger size. Even when compressed via gzip, the source XML retains a meaningful edge.  

> So, why should I save the binary serialization?

Because you don't need to parse the file after the first time. You just need to memory-map it and use it as is.  
This lowers start-up time quite a bit and gives you constant access to its content regardless of size.

> What is the point of a representation which is not mutable? Why cannot I edit the tree?

Technically it is not immutable; however some types of mutations would be very expensive, almost like rebuilding the tree from scratch.  
However, there are some which are quite fast, like reordering attributes, changing value with literals which are already in memory, or adding annotations to mark changes.  
Some of them are implemented by this library, others could be in a nearby future.  

As for why, there is plenty of applications where a mutable tree structure is no longer needed after it has being built.  
This library tries to optimize such cases without introducing features which could compromise them.

> Why pointers are internally represented as relative to a base?

To allow them to be serialized, and still provide random access to the tree regardless of the device or medium on which it is being stored.  
This allows to share annotations for a tree for cheap, even while offloading or distributing computation.

> Why is XML serialization/de-serialization single threaded?

Because they are mostly meant as one-time operations, while the bulk of the operations is going to be on the binary representation.  
There is no trivial way to make XML parsing or serialization "parallel" without making the code involved much more complex.  
At which point it is generally better to waste threads for other tasks if possible.