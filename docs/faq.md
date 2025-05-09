> Why the binary serialization of the XML is bigger compared to the base file?

That is to be expected. XML has little overhead with extra symbols and marker. By comparison, nodes as represented by this library are storing contextual information about their neighbours to speed up navigation.  
This additional pre-computed information is the reason for the extra space needed. Even when compressed via gzip, the source XML has a meaningful edge.  

> So why should I save the binary serialization?

Because you don't need to parse the file any longer. You just need to memory-map it and use it as is. This lowers start-up time quite a bit.

> What is the point of a representation which is not mutable?

It is not immutable, it is just that some mutations would be very expensive, almost like rebuilding the tree from scratch.  
However, there are some mutations which are still viable, like reordering attributes, changing value if the new literal is already in memory, or removing nodes/leaves.  
Some of them are implemented by the library, others could be.  

As for why, there is plenty of applications where there is no need to mutate the tree structure after its build process has ended.  
This library tries to optimize for such cases as much as possible.

> Why pointers are relative?

To allow them to be serialized and still provide random access on the tree regardless of the device or medium on which it is being stored.  
This allows to share annotations for a tree for cheap even in offloaded contexts.