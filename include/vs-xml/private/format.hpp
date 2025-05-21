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


 #if USE_FMT && __has_include(<fmt/core.h>) && __has_include(<fmt/ostream.h>)

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