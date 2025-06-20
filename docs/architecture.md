
## Trees, Documents, Archives

### Tree
A tree is defined by a single root node. There are several types of nodes, reflecting those we have in XML files:

- **element**
- **cdata**
- **text**
- **comment**
- **processing directive**

Each element can have an arbitrary number of attributes and children nodes.  
The rest of them only host a view/span for their content.  
Please, notice that while **text** and **cdata** are kept separate to preserve the original XML structure, once deserialized in memory there is no real difference amongst them.  

There is also a special node type: **markers**.  
These are not part of the basic XML specs, and are only used internally to support downstream algorithms and data structure to annotate, extend or index the tree structure.  

Labels, symbols and strings in general, are not inlined with the tree structure itself, but they are hosted in a separate symbol table.  
In some modes, this can be the original XML file itself, or a special buffer filled in with strings based on some compression method to reuse entries.

### Document
Documents wrappers of tree to allow full XML documents. While an XML document only has one root element, there can be multiple processing directives (starting with `<?xml ... ?>` itself) and comments.  
Documents are just going to enable that.

### Archive
Archives are collections of documents. Each can be optionally given a name.  
Unlike documents, where each can have its own symbol table, there is just one per archive, shared across its nested documents.

## `Class` vs `ClassRaw`

`Class` is a wrapper for `ClassRaw`.  
In exchange for some performance and a slight increase in memory, it keeps better track of contextual information.  
This simplifies the usage of relative string views and the general navigation of the tree structure.  

In practice, you can just use `Class` over `ClassRaw` for virtually any scenario. In case the nice extras provided are really, really not needed, the raw base class will give you a small memory bonus.

## `StorageFor`/`Stored`


## Builders

`Tree`, `Document`, `Archive` all come with their respective builders. They are classes providing an interface to construct the relative object piece by piece.  
To make use of them, include `vs-xml/xxx-builder.hpp`.

## Parser

## Serialization

## Queries

## Binary formats
