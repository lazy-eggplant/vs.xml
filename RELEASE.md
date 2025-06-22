This one is just an intermediate release, don't expect much in terms of fancy new features! The new queries are still cooking in the background.  
Still, this one improves documentation coverage, adds some quality of life features and refactors several bad architectural choices which were dragging since the very start of this project.

## New features

- a new builder for Archive, avoiding most of the boilerplate needed before;
- a new CLI utility to generate a binary archive from a folder
- and its inverse operation;
- `TreeIterator` to replace the current queries based on generators.

## Breaking

- Builder have been split into separate files, so to have a clean separation between *embeddable* and *not embeddable* code.
- `reserve` for builders has been changed to be more extensible and descriptive.