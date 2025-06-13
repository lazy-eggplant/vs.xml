## Memory Layout

### Layout encoding

## Binary serialization

Except for a small header, the binary serialization of a tree is identical to its representation in memory.

### Header format

```mermaid
packet-beta
    title Binary Header (Start)
    0-7: "$"
    8-15: "X"
    16-23: "M"
    24-31: "L"
    32-39: "Format Major"
    40-47: "Format Minor"
    48-55: "Configs"
    56-57: "Endian"
    58-63: "reserved-0"
    64-69: "size (bits) delta_ptr_t"
    70-75: "size (bits) xml_size_t"
    76-81: "size (bits) xml_count_t"
    82-87: "size (bits) xml_enum_size_t"
    88-95: "reserved-1"
    96-111: "documents count"
    112-127: "reserved-2"
    128-191: "symbols size"
```
An array of sections follows. The definition of a section depends on the data types of the build:
```c++
struct __attribute__ ((packed)) section_t{
    struct __attribute__ ((packed)){
        delta_ptr_t  base;      //Aligned to the beginning of the symbols table.
        xml_count_t  length;    //Aligned to base
    } name;
    delta_ptr_t     base;      //Aligned to start_data() (after symbols)
    xml_count_t     length;    //Relative to base
}
```

```mermaid
packet-beta
    title Configs
    0-2: "Symbols"
    3-4: "raw_strings"
    5-6: "allow_comments"
    7-8: "allow_procs"
```

### Multi-doc archives

TBW.

## Indices

TBW.