#pragma once

/**
 * @file unordered_set.hpp
 * @author karurochari
 * @brief Helpers to handle unordered_set portably
 * @date 2025-05-17
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#if VS_XML_USE_GLT == true && __has_include(<gtl/phset.hpp>)

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