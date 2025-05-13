#pragma once

/**
 * @file document.hpp
 * @author karurochari
 * @brief Document wrapper for a tree
 * @date 2025-05-09
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "commons.hpp"
#include "tree.hpp"
#include "builder.hpp"

namespace VS_XML_NS{

template<builder_config_t cfg = {}>
struct DocBuilder : TreeBuilder<cfg>{};

struct DocumentRaw : TreeRaw {
};

struct Document : Tree {
};

}