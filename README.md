> [!WARNING]  
> Ongoing project, the base functionality is ready, but more documentation and some advanced features have not been implemented yet.


This library offers a mostly-compliant[^1] XML parser, tree builder and several related utilities.  
It is not intended as a general purpose library, which means it might not be a good fit for your project.  
Please, read the rest of this readme to know more about its objectives and drawbacks.

## Features

- Support for a schema-less tree structure which can be fully relocated in disk, memory or offloaded devices without impacting its binary representation.
- Linked to the previous point, pointers/iterators based on this tree structure are random access, no need to navigate the tree to reach them.
- Good memory locality of the tree representation, making many operations on sub-trees trivial `memcpy`.
- Configurable memory footprint, the internal representation can decrease size for most of its fields properly run on "lesser" systems or improve cache performance.
- XML serialization and de-serialization.
- Naive support for namespaces[^2].

Non objectives:

- Support for arbitrary editing operations. This library is special-purpose, so only a small number of mutable operations will be supported to keep the rest as fast as possible.
- Extended XML entities, base64, DTD... none of that is needed for the intended target of this library.

## Quick statup

Just use it as any meson dependency by adding a wrap file to this repository.  
Or installing it in your system first and using it as a system dependency.  

You can easily build documents:
```cpp
#include <vs-xml/document.hpp>
#include <iostream>
using namespace xml;

int main(){
  DocBuilder<{.symbols=xml::builder_config_t::COMPRESS_ALL}> bld;
  bld.xml();
  bld.comment("This is a comment!");
  bld.begin("root");
      bld.attr("hello", "world");
      //Children after the attribute block.
      bld.text("This is some text! <escape> sequences will be handled.");
      bld.cdata("This is some cdata! <escape> sequences will be handled.");
  bld.end();
```

Serialize them:
```cpp
  auto document = *bld.close(); //Make sure to handle the return error if present in production code.
  document->print(std::cout);
```

And access the tree structure:
```cpp
  for(auto& it: document.root().children()){
    //... use the tree node in here.
  }
  return 0;
}
```

And more, like reading and saving them from binary files (usually memory mapped).  
Learn more by checking the [examples](./examples/src/).  
Doxygen and the generated documentation can be found in the [github pages](https://lazy-eggplant.github.io/vs.xml/next/) of this project.


## Supported platforms

This library is mostly standalone, but it requires support for the C standard library and a modern version of the C++ standard library.  
As such, it might not be the best option for embedded systems, but the current codebase could be adapted to be.  
Other dependencies are only used for the test-suite and benchmarks, they are not needed to build and install `vs.xml` to your system.

Also, a modern version of C++ should be used.  
I am working with C++23 for development, and I don't really plan on directly supporting older revisions of the language at the expense of code simplicity.  

## Typical applications

Examples of where this library is meant to fit:

### Very big XML files

This library allows serializing XML files into a binary format for fast navigation and information linking.  
It is very easy to do it once, and then load your terabyte big XML as a memory mapped file.  
Since nodes are random accessible via fully relocatable addresses, you will not get the constant penalties of page misses for each nested layer you need to visit.  

### Patches and annotations

Annotating the tree, or even adding small patches on a huge tree can be quite easy[^3]; since addresses are all relative and stable, it is trivial to share your annotations or patches with others.

### Efficient tree building

Tree building is not heap-allocating each node individually, and strings are unescaped in place when parsing a source XML file, so there are no expensive memory allocation needed for that.

### But why?

You can find a FAQ page with some questions being answered. For all the others just ask :). 


## External dependencies

This library is fully standalone (aside from the C/C++ standard libraries).  
However, examples, tests, optional utilities and benchmarks have some dependencies:
- [mio](https://github.com/StrikerX3/mio) a simple way to handle memory mapped files, and pretty much the intended way to use `vs-xml` downstream.
- [nanobench](https://github.com/martinus/nanobench) to perform benchmarks.

## Licence

This library is released as `LGPL3.0`.  
All documentation is under CC4.0 Attribution Share-Alike.  
Examples are CC0, unless something else is specified, but this does not cover datasets for which you will have to individually check.

[^1]: XML 1.0 is covered as a best-effort, but there will be small things where either the official XML standard or this implementation is going to be incompatible or a superset.  
      For more information on compatibility and supported features, please check [here](./docs/features.md) where they are being tracked.
[^2]: Namespaces are supported in the sense that the namespace is split from the element or attribute name if present, but its handling, validation or whatever is left to the user.

[^3]: However, using such patches would require a downstream implementation of wrapper classes.