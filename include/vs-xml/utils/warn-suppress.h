#pragma once

// Detect compiler
#if defined(__clang__)
  #define COMPILER_CLANG 1
#elif defined(__GNUC__)
  #define COMPILER_GCC   1
#else
  #error "Unsupported compiler for warning suppression"
#endif

// Helper macros to turn a pragma argument into an _Pragma
#define DO_PRAGMA(x) _Pragma(#x)

// Save/restore diagnostic state
#if COMPILER_CLANG
  #define WARN_PUSH              DO_PRAGMA(clang diagnostic push)
  #define WARN_IGNORE(warning)   DO_PRAGMA(clang diagnostic ignored warning)
  #define WARN_POP               DO_PRAGMA(clang diagnostic pop)
#elif COMPILER_GCC
  #define WARN_PUSH              DO_PRAGMA(GCC diagnostic push)
  #define WARN_IGNORE(warning)   DO_PRAGMA(GCC diagnostic ignored warning)
  #define WARN_POP               DO_PRAGMA(GCC diagnostic pop)
#endif

