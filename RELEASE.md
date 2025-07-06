## Breaking

- `DocBuilder` renamed as `DocumentBuilder` to be more consistent in naming.
- Removed `path` functions from XML entities, leftovers from the very early versions;  
  this functionality can now be trivially replaced by user-defined functions, since the rest of the interface is complete.

## Features

Introduced `Tree::visit` and `TreeRaw::visit` to implement a slightly different and more flexible visitor pattern when compared to the one recently added iterator.  
They are both based on `private/(wrp-)visit.hpp`, which is not publicly exposed (for now).  

New `print` functions have been introduced for trees et similia. It no longer uses `std::print` due to the awful overhead and additional memory allocations. `fmt` had no such issue to be honest.