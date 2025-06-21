Intermediate release, there will not be much in terms of fancy features.  
Still, this one improves the documentation, and refactors several bad architectural choices which were dragging since the start of this project.

## New features

~~A new builder for Archive, avoiding the need of boilerplate.~~
~~New CLI tools to generate a binary archive from a folder and the reverse operation.~~

## Breaking

- Splitting builders into separate files to have a clean separation between embeddable and not embeddable code.
- `reserve` for the builders changed interface to ensure more fields can be added with descriptive label.