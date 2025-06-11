A subset of features of this library is embedded-friendly or there is an active effort in making them such.  
This usually implies:

- It operates without requiring exceptions (mostly replaced by `std::optional` and `std::expected`).
- It does not require dynamic memory allocations, or when they are needed, allocation events are kept to a minimum. More on this later.
- The tree structures can be scaled down considerably in size, to better match type sizes which are compatible with the target system.
- Lightweight usage of the STL, no runtime features needed. In many cases, the STL can be replaced with different libraries via configuration flags.

## Usage

A basic build could look like this:

```bash
meson setup build-emb --native-file=./platforms/gcc-embedded.ini -Dnoexcept=true -Dutils=false
```

It is important to enable the `noexcept` flag and disable `utils` alongside any other optional module. 

## Features

### 🟢 Features optimized for embedded
- `TreeRaw`/`Tree` general usage
- `DocumentRaw`/`Document` general usage
- `ArchiveRaw`/`Archive` general usage
- The XML parser when `.raw_strings=true`, however wraps builders which are not fully optimized yet.
- Memos/notes/indices can all be implemented externally, as long as you have a proper library for containers `vs.xml` will not get in your way.

### 🟠 Features planned for embedded
- `TreeBuilder` & `DocumentBuilder`. Right now they own their storage, unable to just work on externally defined ones and memory allocations are needed to build archives.  
  It is possible to reserve space limiting allocations, but they cannot be fully removed.
- The XML serializer when `.raw_strings=true`, it is still using functions which are not optimized, but their replacement has been implemented already. It also assumes to operate on a stream which is not great.
- The `QueryBuilder`, same notes as for the other builders.
- Queries. Right now they are not good due to the high number of dynamic allocations needed. They could be trivially removed for the most part, but the whole system is being refactored to be stack-based and consume less memory overall.

### 🔴 Features not planned for embedded
- The utilities shipped alongside this library are not meant for embedded usage.
- Most of the test-suite. Embedded will feature its own subset of tests and benchmarks.

## Storage handling

If you want to avoid memory allocations, please make use of the original Tree/Document/Archive in place of their wrappers in `xml::stored` as you might see suggested in several examples.  
The originals don't own their memory, so you can externally manage it as you prefer.