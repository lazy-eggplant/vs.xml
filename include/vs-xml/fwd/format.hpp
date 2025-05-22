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