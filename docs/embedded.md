
Most usage of `libc++` is only related to header features, so it might be possible to introduce alternatives:
- `libfmt` to cover `std::format` and `std::print`.
- Some implementation for `std::vector`, `std::stack` and `std::unordered_map`.
