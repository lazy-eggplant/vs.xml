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

#if VS_XML_USE_FMT && __has_include(<gtl/phset.hpp>)

#include <gtl/phset.hpp>

namespace xml{
    template<typename... args>
    using unordered_set = gtl::flat_hash_set<args...>;
}

#else

#include <unordered_set>

namespace xml{
    template<typename... args>
    using unordered_set = std::unordered_set<args...>;
}

#endif