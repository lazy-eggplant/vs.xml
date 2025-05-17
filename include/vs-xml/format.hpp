#pragma once

#if USE_FMT && __has_include(<fmt/core.h>)
#include <fmt/core.h>
namespace xml{
using fmt::format;
}
#else
#include <format>
namespace xml{
using std::format;
}

#endif