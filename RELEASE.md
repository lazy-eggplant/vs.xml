## v0.3.1

Hardening + binary format bump + query redesign + C API. Full notes in
`docs/releases/v0.3.1.md`.

- Hardened binary loading (bounds and alignment validation).
- Parser and serializer correctness; pretty-printing.
- Explicit query builder (`xml::query::query_t`).
- C bindings (`vs-xml/c.h`).
- xmake build; Meson and CMake removed; `fmt` required.
