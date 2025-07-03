## Breaking

- `DocBuilder` renamed as `DocumentBuilder` to be more consistent in naming.

## Features

Introduced `Tree::visit` and `TreeRaw::visit` to implement a slightly different visitor pattern compared to the recently added iterators.  
Introduced a new `print2` function for trees and derived siblings, to provide a not recursive variant of `print` which does not grow on stack based on the depth of the tree.  
Not tested yet, but it will deprecate `print`.