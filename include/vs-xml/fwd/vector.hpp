#pragma once

/**
 * @file vector.hpp
 * @author karurochari
 * @brief Helpers to handle vector portably
 * @date 2025-06-02
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#if VS_XML_USE_GLT == true && __has_include(<gtl/vector.hpp>)

#include <gtl/vector.hpp>

namespace xml{
    template<typename... args>
    using vector = gtl::vector<args...>;
}

#else

#include <vector>

namespace xml{
    template<typename... args>
    using vector = std::vector<args...>;
}

#endif