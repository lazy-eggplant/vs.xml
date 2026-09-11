# Third-party software

`vs.xml` itself is distributed under the LGPL-3.0 license (see
[LICENCE.md](./LICENCE.md)). Build and test dependencies are listed below.

## Runtime dependencies

| Component | Used for | License |
|-----------|----------|---------|
| [fmt](https://github.com/fmtlib/fmt) | formatting and printing (required) | MIT |
| [gtl](https://github.com/greg7mdp/gtl) | optional alternative containers | Apache-2.0 |

## Build and test only

These are pulled in by optional xmake targets (tools, benchmarks) or the
test-suite only; the core library does not need them.

| Component | Used for | License |
|-----------|----------|---------|
| [mio](https://github.com/mandreyel/mio) | memory-mapped file tools/demos | MIT |
| [linenoise](https://github.com/antirez/linenoise) | line editing in the query tool | BSD-2-Clause |
| [nanobench](https://github.com/martinus/nanobench) | benchmarks | MIT |
| [pugixml](https://pugixml.org/) | comparative benchmarks | MIT |
