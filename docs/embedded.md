A subset of features of this library is embedded-friendly or there is an active effort in making them such.  
This usually implies:

- It operates without requiring exceptions (mostly replaced by `std::optional` and `std::expected`).
- It does not require dynamic memory allocations, or when they are needed, allocation events are kept to a minimum. More on this later.
- The tree structures can be scaled down considerably in size, to better match type sizes which are compatible with the target system.
- Lightweight usage of the STL, no runtime features needed. In many cases, the STL can be replaced with different libraries via configuration flags.

## Usage

A basic build could look like this:

```bash
xmake f -m release --no_except=y -y
xmake -y
```

It is important to enable the `--no_except` flag and avoid the optional targets
(tools, examples, benchmarks, tests).

## Features

### 🟢 Features optimized for embedded
- `TreeRaw`/`Tree` general usage
- `DocumentRaw`/`Document` general usage
- `ArchiveRaw`/`Archive` general usage
- The XML parser when `.raw_strings=true`, however it wraps builders which are not fully optimized yet.
- The XML serializer when `.raw_strings=true`.
- Memos/notes/indices can all be implemented externally, as long as you have a proper library for containers `vs.xml` will not get in your way.

### 🟠 Features planned for embedded
- `TreeBuilder`, `DocumentBuilder`, `ArchiveBuilder`. Right now they encapsulate their own storage, unable to just work on externally defined containers, so we cannot externally handle memory allocations.  
  It is possible to reserve space and so limiting the number of allocations, but they cannot be fully removed as it is.
- Queries. The explicit builder still allocates its step list and result containers; these could be made arena-based to remove the remaining allocations.

### 🔴 Features not planned for embedded
- The utilities shipped alongside this library are not meant for embedded usage.
- Most of the test-suite. Embedded targets will feature their own subset of tests and benchmarks as the coding style is quite different.

## Storage handling

If you want to avoid memory allocations, please make use of the original Tree/Document/Archive in place of their wrappers in `xml::stored` as you might see suggested in several examples.  
The originals don't own their memory, so you can externally manage it yourself as desired.