## Breaking

- `DocBuilder` renamed as `DocumentBuilder` to be more consistent in naming.
- Removed `path` functions from XML entities, leftovers from the very early versions;  
  this functionality can now be trivially replaced by user-defined functions, since the rest of the interface is complete.
- The binary interface of the tree changed. Again. But it is for good reasons! We optimized away one of the biggest fields in nodes, saving a significant amount of memory.  
  Technically this change prevents out of order nodes in the memory layout, but this was just a side-effect extra, not something intended.

## Features

Introduced `Tree::visit` and `TreeRaw::visit` to implement a more flexible visitor pattern when compared to the one recently added iterator-based approach.  
They are both based on `private/(wrp-)visit.hpp`, which is not publicly exposed (for now).  

New `print` functions have been introduced for trees, based on the visitor pattern. It no longer uses `std::print` due to the awful overhead and additional memory allocations. `fmt` had no such issue to be honest.  
The legacy version has been optimized as well: it is now called `print_fast` and still uses simple recursion to get a signifiant edge on performance; however, be mindful of stack overflows if working with stack intensive applications or badly nested trees.