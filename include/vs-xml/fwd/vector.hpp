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

#include <vs-xml/commons.hpp>

#if VS_XML_USE_GTL == true && __has_include(<gtl/vector.hpp>)

#include <gtl/vector.hpp>

namespace VS_XML_NS{
    template<typename... args>
    using vector = gtl::vector<args...>;
}

#else

#include <vector>

namespace VS_XML_NS{
    template<typename... args>
    using vector = std::vector<args...>;
}

#endif