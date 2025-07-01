## Breaking

- `DocBuilder` renamed as `DocumentBuilder` to be more consistent in naming.

## Features

Introduced `Tree::visit` and `TreeRaw::visit` to implement a slightly different visitor pattern compared to the recently added iterators.  
It will be used to refactor `print/print_h` used for serialization and Queries.