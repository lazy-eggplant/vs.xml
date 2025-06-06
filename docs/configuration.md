
For performance reasons, most configuration flags are defined at compile-time.  
A single application will have to build and link different versions of this library if they want different configurations, but that scenario is extremely unlikely.

## Meson project

- `tests` defaults to false. Change to enable test-suite.
- `benchmarks` defaults to false. Change to enable benchmarks.
- `examples` defaults to false. Change to enable examples.
- `utils` defaults to true. Change to compile the extra system utilities.
- `use_fmt` defaults to true. Disable it if you want to force using `std::print`/`std::format` instead, but be mindful [they are much slower](https://github.com/lazy-eggplant/vs.xml/issues/8).
- `use_gtl` defaults to false. Enable it to use alternative STL-like containers with better performance and memory-mappability (is that even a word?).
- `noexcept` defaults to false. Used to disable exceptions from the build. Often needed for embedded or offloaded targets.

## Defines

- `VS_XML_NS` defaults to `xml`. Changes the namespace if needed, to avoid collisions.
- `VS_XML_NO_ASSERT` to remove assertions from this library.
- `VS_XML_NO_EXCEPT` to remove exceptions from this library as much as possible.
- `VS_XML_LAYOUT` is used to control the memory layout (defaults to 0). Current profiles:
    - `0` Normal/aligned
    - `1` Compact (mostly compatible with real world documents, less cache-misses, less space on disk)

## Data types & Layout


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
> This goes beyond the configurability granted via macros, and forking this library is likely the best option.


## Builder

Builders are parametrized by a configuration structure and initialized in ways which defines its behaviour and ownership of data.

### Configuration fields

- `allow_comments` if true allows comments to be appended in the tree, else they are silently skipped (but can still return errors for validation)
- `allow_procs` if true allows processing directives to be appended in the tree, else they are silently skipped (but can still return errors for validation)
- `symbols`:
    - `EXTERN_ABS` using full memory in absolute position; the generated tree cannot be saved as binary.
    - `EXTERN_REL` can be saved as binary, but it requires the tree to bind a symbol table later when constructed.
    - `OWNED` no compression of symbols, but they are owned.
    - `COMPRESS_LABELS` only compressing symbols which are used for tag and prop labels (and namespaces).
    - `COMPRESS_ALL` all symbols are compressed.
    - ~~`COMPRESS_CUSTOM`~~ not implemented yet. Used to determine the usage of a custom compression algorithm, offloading it from the library.  
      Useful to specify your own for embedded systems or to implement more expensive but compressed representations.
- `raw_strings` if true, strings are assumed to be preserved as is when serialized or deserialized from XML.  
  Make sure you are manually escaping when building the tree. Comparisons on the other hand will handle the conversion automatically when not using the `xxxRaw` versions of the library classes.
