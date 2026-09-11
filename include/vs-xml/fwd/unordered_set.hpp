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

#include <vs-xml/commons.hpp>

#if VS_XML_USE_GTL == true && __has_include(<gtl/phset.hpp>)

#include <gtl/phset.hpp>

namespace VS_XML_NS{
    template<typename... args>
    using unordered_set = gtl::flat_hash_set<args...>;
}

#else

#include <unordered_set>

namespace VS_XML_NS{
    template<typename... args>
    using unordered_set = std::unordered_set<args...>;
}

#endif