## Memory Layout

The memory layout for the tree structure used by this library has the following properties:

- It is based on a contiguous memory slice.
- It is built in-place, as such children are placed directly after their parent.
- Attributes are immediately stored after a node.
- Pointers between nodes in the tree are not defined as absolute, but relative to the address of the current one.
- All strings are represented as string views over a common base offset which is valid for the whole tree.

The structure of the various nodes is based on the size of three data types:

- `delta_ptr_t` is the type of relative pointers. Realistically, 16bits is plenty enough for most realistic scenarios with XML files humans can handle.
- `xml_size_t` is the type used to represent the size of nodes in bytes. It should be as high as the max size in byte a tree can have. 16bits is also often sufficient.
- `xml_count_t` is the type used to represent the count of things. Like the number of attributes, or number of bytes in a string. Its size is mostly determined by the maximum length of strings_views. Depending on the application as low as 8bit could be enough, 16bit a more reasonable default.
- `xml_enum_size_t` is the size of enums when stored. 8bit is plenty enough.

> [!NOTE]  
> If you have very specific needs for a compact memory layout, probably more suitable for embedded applications, you will have to manually override parts of this code and possibly work with bit-fields.
> This goes beyond the configurability granted via macros, and forking this library is likely the best solution.

## Binary serialization

Except for a small header, the binary serialization of a tree is identical to its representation in memory.

### Binary format

TBW.
