> [!WARNING]  
> Ignore this document, it is just for internal reference for features which are still in the "does it make any sense" phase, and are unlikely to be explored in this project.

There are several types of fields:
- Inline, fixed size final value. Not compressed, each row has an entry in the field container.
- Indirect, variable size final value. Not compressed, each row has an entry in the field container. Storing slices for data stored on some allocators.
- Palette index, fixed/variable size final value. Not compressed, each row stores the index of the value in a field container. The field container in this case operates as a special palette, linked to either fixed size or variable size content.

Any of the above can be in the compressed variant, used for sparse content. Each value is assumed to be a given default (either fixed value or computed) unless specified. The underlying storage is performed by a binary balanced tree of sort or a hash map.

Bitfields fields allow for multiple subfields to share (when masked) the same contiguous slice of memory. Null for each field needing it is implemented as an aggregate bitfield covering all the other fields at once.

```c
//https://www.reddit.com/r/C_Programming/comments/sixefk/c_object_notation_interchange_format/

table_t table = {
    .name = "table-a",
    .meta = {
        .author = "Banana magica",
        .desc = "Text here"
        .tags = {"tag-0", "tag-1"},
        .insert_mode = FIST_EMPTY, //alternative CIRCULAR_BUFFER, FIXED_BUFFER, GROW
        .delete_mode = DEFAULT, //Alternative DISABLED
        .update_mode = DEFAULT, //Alternative DISABLED
    },
    .fields = {
        {
            .name = "field-0",
            .type = int(8),
            .nullable = true,
            .desc = "Long description of this field"
        }, 
        {
            .name = "field-1",
            .type = int(4),
            .nullable = true,
            .defval = int(4,"44")
        },
        {
            .name = "field-2",  //field-1 and field-2 will probably end up in the same storage space masked.
            .type = int(4),
            .nullable = false,
        },
        {
            .name = "field-3",  //field-1 and field-2 will probably end up in the same storage space masked.
            .type = varchar(64),
            .nullable = false,
            .defval = "empty"
            .sparse = true,
        },
        {
            .name = "field-4",  //field-1 and field-2 will probably end up in the same storage space masked.
            .type = text(),
            .nullable = false,
        }
    },
    .indeces = {
        {
            .type = primary_key(),
            .name = "primary_key",
            .fields = {"field-1", "field-2"},
        },
        {
            .type = unique(),
            .name = "primary_key",
            .fields = {"field-3"},
        },
    }
};

foreign_t fkey = 
    {
        .name = "foreign_1"
        .src = {
            .table = "table-name-1",
            .fields = { "field-0", "field-1" }
        }
        .dst = {
            .table = "table-name-2",
            .fields = { "field-a", "field-b" }
        }
    }
    
```


Special row-level bitfields:

```cpp
struct row_controls_t{
    uint8_t in_foreign: 1;
    uint8_t allow_read: 1;
    uint8_t allow_write: 1;
    uint8_t allow_delete: 1;
    uint8_t allow_read: 1;
    uint8_t stratch_bits[3]: 1;
    uint8_t used: 1;
};
```