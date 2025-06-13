> [!WARNING]  
> Ignore this document, it is just for internal reference for features which are still in the "does it make any sense" phase, and are unlikely to be explored in this project.

Fields can be in one of the following configurations:

- Fixed size (inline). Not compressed. Each row has its own entry.
- Variable size. Indirect. Not compressed. Each row has its own entry. Size of the field representing the pointer will scale up with the row number.
- Fixed/Varialbe size. Indirect. Compressed. Each row has its own pointer, but values in the symbols table can be shared. Size of the field representing the pointer will scale up with the values number.