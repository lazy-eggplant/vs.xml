#pragma once

#if USE_FMT && __has_include(<fmt/core.h>)
#include <fmt/core.h>
#include <fmt/ostream.h>
namespace xml{
using fmt::format;
using fmt::print;
}
#else
#include <format>
#include <print>
namespace xml{
using std::format;
using std::print;
}

#endif