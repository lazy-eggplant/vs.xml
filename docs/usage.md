## Installing

This library is meant to be used either as a system dependency, or built as an xmake subproject/package.  
The second option grants more flexibility if you need control over the memory layout or other configuration flags.  

To our knowledge, this library is not distributed by any distribution. To install it on your system you will probably want to write something like:

```sh
xmake f -m release          #Add flags such as --with_tools=y to build the command line utilities.
xmake
xmake install -o /usr/local #Set the desired prefix.
```

A simplified makefile is available, which sets some reasonable defaults and wraps xmake.

Tarballs will also be offered for some distributions as part of future releases.  

## As a dependency

With [xmake](https://xmake.io):

```lua
add_requires("vs.xml")
```

Or, from a local checkout, point xmake at the project directly.

### Configuration
Read [this](./configuration.md) document about configuration.

## Examples

Please, refer to the `examples` folder to see this library in action.
