#pragma once

/**
 * @file format.hpp
 * @author karurochari
 * @brief Helpers to handle unordered_map portably
 * @date 2025-05-17
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#if VS_XML_USE_FMT && __has_include(<gtl/phmap.hpp>)

#include <gtl/phmap.hpp>

namespace xml{
    template<typename... args>
    using unordered_map = gtl::flat_hash_map<args...>;
}

#else

#include <unordered_map>

namespace xml{
    template<typename... args>
    using unordered_map = std::unordered_map<args...>;
}

#endif