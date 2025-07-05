#pragma once

/**
 * @file format.hpp
 * @author karurochari
 * @brief Helpers to handle format/print portably
 * @date 2025-05-17
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#if VS_XML_USE_FMT == true && __has_include(<fmt/core.h>)

#include <fmt/core.h>

namespace xml{
    using fmt::format;
}

#else

#if VS_XML_USE_FMT == true && !__has_include(<fmt/core.h>)
    #warning "Unable to use fmt, header missing"
#endif

#include <format>

namespace xml{
    using std::format;
}

#endif