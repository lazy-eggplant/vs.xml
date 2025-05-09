#pragma once

#ifndef VS_XML_NS
#define VS_XML_NS xml
#endif

#include <source_location>
#include <iostream>
//#include <stacktrace>

namespace VS_XML_NS{



#if __cplusplus >= 202002L
 inline void modern_assert(bool condition, const char* errorMessage,
                             std::source_location location = std::source_location::current()) {
    if (!condition) {
        std::cerr << "Error: " << errorMessage << "\n"
                  << "File: " << location.file_name() << "\n"
                  << "Function: " << location.function_name() << "\n"
                  << "Line: " << location.line() << "\n"
                  << "Column: " << location.column() << std::endl;
/* Not supported yet 
#if __cplusplus >= 202300L
        std::cerr << "Stack trace:\n";
        for (const auto& frame : std::stacktrace::current())
            std::cerr << frame << std::endl;
#endif
*/
        throw std::runtime_error(errorMessage);
    }
}
#endif

}