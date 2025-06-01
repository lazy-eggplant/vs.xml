A subset of features of this library is embedded-friendly or there is an active effort in making them such.  
This usually implies:

- It operates without requiring exceptions (mostly replaced by `std::optional` and `std::expected`).
- It does not require dynamic memory allocations, or when they are needed, allocation events are kept to a minimum. More on this later.
- The tree structures can be scaled down considerably in size, to better match type sizes which are compatible with the target system.
- Lightweight usage of the STL, no runtime features needed. In many cases, the STL can be replaced with different libraries via configuration flags.

## Features optimized for embedded
- `TreeRaw`/`Tree` general usage
- `DocumentRaw`/`Document` general usage
- The XML parser when `.raw_strings=true`, however wraps builders which are not fully optimized yet.
- Memos/notes/indices can all be implemented externally, as long as you have a proper library for containers `vs.xml` will not get in your way.

## Features planned for embedded
- `ArchiveRaw`/`Archive` are currently using the heap to store a redundant vector just to keep the code simple. But it is first on list for removal.
- `TreeBuilder` & `DocumentBuilder`. Right now they own their storage, unable to just work on externally defined ones.  
  It is possible to reserve space limiting allocations, but they cannot be fully removed.
- The XML serializer when `.raw_strings=true`, it is still using functions which are not optimized, but their replacement has been implemented already. It also assumes to operate on a stream.
- The `QueryBuilder`, same notes as for the other builders.
- Queries. Right now they are not good due to the high number of dynamic allocations needed. They could be trivially removed for the most part, but the whole system is being refactored to be stack-based and consume less memory overall.

## Features not planned
- The utilities shipped alongside this library are not meant for embedded usage.