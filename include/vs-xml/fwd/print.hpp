#pragma once

/**
 * @file print.hpp
 * @author karurochari
 * @brief Helpers to handle format/print portably
 * @date 2025-05-17
 * 
 * @copyright Copyright (c) 2025
 * 
 */


#if VS_XML_USE_FMT == true &&  __has_include(<fmt/ostream.h>)

#include <fmt/ostream.h>

namespace xml{
    using fmt::print;
}

#else

#if VS_XML_USE_FMT == true && !__has_include(<fmt/ostream.h>)
    #warning "Unable to use fmt, header missing"
#endif

#include <print>

namespace xml{
    using std::print;
}

#endif