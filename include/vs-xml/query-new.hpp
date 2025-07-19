#pragma once

/**
 * @file query2.hpp
 * @author karurochari
 * @brief Queries on Tree/Document.
 * @date 2025-06-22
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <string_view>
#include <vs-xml/commons.hpp>

namespace VS_XML_NS{
namespace query2{

struct Hooks{
    //Propagate the request of binding a variable to a value
    void (*bind  )(size_t uid, std::string_view name, size_t data_ref, void* ctx);
    //Hook for when a match if finally accepted.
    void (*accept)(size_t uid, void* ctx);
    void (*refuse)(size_t uid, void* ctx);
    void (*fork  )(size_t uid, void* ctx);
    //pop/push?
};

}
}