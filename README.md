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
- An efficient engine to perform queries on a document, all based on lazy evaluation.
- XML serialization and de-serialization.
- Naive support for namespaces[^2].

Non objectives:

- Support for arbitrary editing operations. This library is special-purpose, so only a small number of mutable operations will be supported to keep the rest as fast as possible.
- Extended XML entities, base64, DTD... none of that is needed for the intended target of this library.
- In general, being fully XML compliant.

## Quick startup

Just use it as any meson dependency by adding a wrap file to this repository.  
Or installing it in your system first and using it as a system dependency.  

Full code in the [examples folder](./examples/src/readme.cpp). You can easily build documents:
```cpp
#include <vs-xml/document.hpp>
#include <vs-xml/filters.hpp>
#include <vs-xml/query.hpp>

#include <iostream>
#include <ranges>
#include <print>

using namespace xml;

int main(){
  DocBuilder<{.symbols=xml::builder_config_t::COMPRESS_ALL}> bld;
  bld.xml();
  bld.comment("This is a comment!");
  bld.begin("base-node");
      bld.attr("hello", "world");
      //Children after the attribute block.
      bld.text("This is some text! <escape> sequences will be handled.");
      bld.cdata("This is some cdata! <escape> sequences will be handled.");
  bld.end();
```

Serialize them:
```cpp
  auto document = *bld.close(); //Make sure to handle the return error if present in production code.
  document.print(std::cout, {/*serialization configuration*/});
```

Access the tree structure:
```cpp
  //Show comments only
  for(auto& it: document.root().children() | std::views::filter([](auto it){return it.type()==xml::type_t::COMMENT;})){
    std::print("{}\n",it.value().value_or("-- Empty node --"));
  }

  //Example of a helper filter (defined in `vs-xml/filters.hpp`)
  for(auto& it: document.root().children() | filters::name("base-node")){
    std::print("{}\n",it.value().value_or("-- Empty node --"));
  }
```

Perform queries:
```cpp
  auto query_a = xml::query::query_t{}/"base-node"/xml::query::accept();

  for(const auto& t : document.root() & query_a){
      std::print("{} @ {}\n", (int)t.type(), t.addr());
  }

  return 0;
}
```

And more, like reading and saving them from binary files (usually memory mapped).  
Learn more by checking the [examples](./examples/src/).  
Doxygen and the generated documentation can be found in the [github pages](https://lazy-eggplant.github.io/vs.xml/next/) of this project.


## Supported platforms

This library is mostly standalone, but it requires support for the C standard library and a modern version of the C++ standard library.  
I am working with C++23 for development, and I don't really plan on directly supporting older revisions of the language at the expense of code simplicity.  
Other dependencies are only used for the test-suite and benchmarks, they are not needed to build and install `vs.xml` to your system.  

Still, many of its features can be replaced by alternative libraries which are more suitable for embedded systems.  
You can track our efforts for embedded support in the [dedicated page](./docs/embedded.md) and [tracker](https://github.com/lazy-eggplant/vs.xml/issues/9).  

We also try our best to ensure this library (or a subset of it) can properly work on offloaded targets like GPUs via [OpenMP](https://www.openmp.org/).  
To use `vs.xml` to its fullest extent, make sure you have a kernel image with unified memory access, and GPUs capable of that.  
We have a [dedicated page](./docs/offloading.md) and [tracker](https://github.com/lazy-eggplant/vs.xml/issues/15).  

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

You can find a [FAQ page](./docs/faq.md) with some questions being answered. For all the others just ask :). 


## External dependencies

This library is fully standalone (aside from the C/C++ standard libraries).  
However, examples, tests, optional utilities and benchmarks have some dependencies:

- [mio](https://github.com/StrikerX3/mio) a simple way to handle memory mapped files, and pretty much the intended way to use `vs-xml` downstream.
- [nanobench](https://github.com/martinus/nanobench) to perform benchmarks.
- [pugixml](https://pugixml.org/) since it is the one I am testing against in benchmarks; these two libraries are very different in scope, so comparative benchmarks can only be marginal and not very useful.

Also, parts of the standard library can be replaced to gain some sweet benchmarking numbers (or to gain additional functionality):
- [fmt](https://github.com/fmtlib/fmt) as an optional replacement of `std::format` and `std::print`, as their performance by comparison are [trash](https://github.com/lazy-eggplant/vs.xml/issues/8).
- [gtl](https://github.com/greg7mdp/gtl) as an optional replacement for some containers in the standard C++ library, with focus on performance and a serializable memory representation.

## Licence

This library is released as `LGPL3.0`.  
All documentation is under `CC4.0 Attribution Share-Alike`.  
Examples are `CC0`, unless something else is specified, but this does not cover datasets for which you will have to individually check.

[^1]: XML 1.0 is covered as a best-effort, but there will be small things where either the official XML standard or this implementation is going to be incompatible or a superset.  
      For more information on compatibility and supported features, please check [here](./docs/features.md) where they are being tracked.
[^2]: Namespaces are supported in the sense that the namespace is split from the element or attribute name if present, but its handling, validation or whatever is left to the user.

[^3]: However, using such patches would require a downstream implementation of wrapper classes.