#pragma once

/**
 * @file assert.hpp
 * @author karurochari
 * @brief Utility for assertions 
 * @date 2025-05-17
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <cstdlib>
#include <stdexcept>
#include <source_location>
//#include <iostream>
//#include <stacktrace>

namespace VS_XML_NS{



#if __cplusplus >= 202002L
 inline void modern_assert(bool condition, const char* errorMessage,
                             std::source_location location = std::source_location::current()) {
    if (!condition) {
        /*std::cerr << "Error: " << errorMessage << "\n"
                  << "File: " << location.file_name() << "\n"
                  << "Function: " << location.function_name() << "\n"
                  << "Line: " << location.line() << "\n"
                  << "Column: " << location.column() << std::endl;*/
/* Not supported yet 
#if __cplusplus >= 202300L
        std::cerr << "Stack trace:\n";
        for (const auto& frame : std::stacktrace::current())
            std::cerr << frame << std::endl;
#endif
*/
        #if VS_XML_NO_EXCEPT != true
        throw std::runtime_error(errorMessage);
        #else
        std::exit(1);
        #endif
    }
}
#endif

}