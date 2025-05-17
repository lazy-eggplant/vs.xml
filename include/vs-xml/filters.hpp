#pragma once

/**
 * @file filter-utils.hpp
 * @author karurochari
 * @brief A library of helpers for filters to use in `children` and `attrs`
 * @date 2025-05-16
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "commons.hpp"
#include <ranges>
#include <string_view>

namespace VS_XML_NS{

/**
 * @brief Namespace for all filters to simplify DOM navigation, offering a small set of common requirements for XML elements.
 * 
 */
namespace filters{
    /**
     * @brief Only pick nodes for which the namespace is set to `sv`
     * 
     * @param sv the namespace to filter
     * @return constexpr auto a filter to pipe after `children` or as argument.
     */
    constexpr inline auto ns(std::string_view sv){
        return std::views::filter([sv](auto& it){return (it.ns().value_or("") == sv);});
    } 

    /**
     * @brief Only pick nodes for which the name is set to `sv`
     * 
     * @param sv the namespace to filter
     * @return constexpr auto a filter to pipe after `children` or as argument.
     */
    constexpr inline auto name(std::string_view sv){
        return std::views::filter([sv](auto& it){return (it.name().value_or("") == sv);});
    } 

    /**
     * @brief Only pick nodes for which the name is set to `sv`
     * 
     * @param sv the namespace to filter
     * @return constexpr auto a filter to pipe after `children` or as argument.
     */
     constexpr inline auto value(std::string_view sv){
        return std::views::filter([sv](auto& it){return (it.value().value_or("") == sv);});
    } 

    /**
     * @brief Only pick nodes for which the name has prefix `sv`
     * 
     * @param sv the namespace to filter
     * @return constexpr auto a filter to pipe after `children` or as argument.
     */
    constexpr inline auto prefix(std::string_view sv){
        return std::views::filter([sv](auto& it){return (it.name().value_or("").starts_with(sv));});
    } 

    /**
     * @brief Filter only those which are XML elements.
     * 
     * @return constexpr auto a filter to pipe after `children` or as argument.
     */
    constexpr inline auto is_element(){
        return std::views::filter([](auto& it){return it.type()==type_t::ELEMENT;});
    } 

    /**
     * @brief Filter only those which are XML text elements (proper TEXT and CDATA both).
     * 
     * @return constexpr auto a filter to pipe after `children` or as argument.
     */
    constexpr inline auto is_text(){
        return std::views::filter([](auto& it){return it.type()==type_t::CDATA || it.type()==type_t::TEXT;});
    } 
}

}