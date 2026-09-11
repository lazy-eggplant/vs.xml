#pragma once

/**
 * @file unordered_map.hpp
 * @author karurochari
 * @brief Helpers to handle unordered_map portably
 * @date 2025-05-17
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <vs-xml/commons.hpp>

#if VS_XML_USE_GTL == true && __has_include(<gtl/phmap.hpp>)

#include <gtl/phmap.hpp>

namespace VS_XML_NS{
    template<typename... args>
    using unordered_map = gtl::flat_hash_map<args...>;
}

#else

#include <unordered_map>

namespace VS_XML_NS{
    template<typename... args>
    using unordered_map = std::unordered_map<args...>;
}

#endif