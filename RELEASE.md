## Breaking

- `DocBuilder` renamed as `DocumentBuilder` to be more consistent in naming.
- Removed `path` functions from XML entities, leftovers from the very early versions; this functionality can now be trivially replaced by user-defined functions as the rest of the interface is complete.

## Features

Introduced `Tree::visit` and `TreeRaw::visit` to implement a slightly different and more flexible visitor pattern compared to the one recently added as iterator.  
They are both based on `private/(wrp-)visit.hpp`, which is not publicly exposed but if one cares about getting maximum performance out of a visitor they are to be used.  

Introduced a new `print2` function for trees and derived siblings, to provide a not recursive variant of `print` which does not grow on stack based on the depth of the tree.  
Not tested yet, but it will deprecate `print`.