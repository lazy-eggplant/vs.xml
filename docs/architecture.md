## `StorageFor`/`Stored`

## Trees, Documents, Archives

## `Class` vs `ClassRaw`

`Class` is a wrapper for `ClassRaw`.  
In exchange for some performance and fatter objects on the stack, it keeps better track of contextual information, simplifying the usage of string views and navigation of the tree structure.  
In practice, you can just use `Class` over `ClassRaw` for virtually all scenarios, but if those nice extras are not needed, sticking to the raw base class will free a bit of space.

## Builders

## Parser

## Serialization

## Queries

## Binary formats
