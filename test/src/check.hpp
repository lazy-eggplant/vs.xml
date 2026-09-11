#pragma once

#include <cstdio>
#include <cstdlib>

// Always-active check, independent of NDEBUG (which release builds set).
#define CHECK(cond)                                                            \
    do {                                                                       \
        if(!(cond)) {                                                          \
            std::fprintf(stderr, "CHECK failed: %s (%s:%d)\n", #cond, __FILE__, __LINE__); \
            std::abort();                                                      \
        }                                                                      \
    } while(0)
