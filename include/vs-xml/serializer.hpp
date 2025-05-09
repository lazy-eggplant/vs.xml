#pragma once

/**
 * @file serializer.hpp
 * @author karurochari
 * @brief Implementation of serialization/deserialization helpers
 * @date 2025-05-01
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <cassert>

#include <expected>

#include <optional>
#include <variant>
#include <string>
#include <string_view>

namespace VS_XML_NS{
namespace serialize{
//https://stackoverflow.com/questions/1091945/what-characters-do-i-need-to-escape-in-xml-documents
//An attempt will be made to keep the input string_view, unless escaping is needed.

typedef std::optional<std::variant<std::string,std::string_view>> ret_t;

std::string_view validate_xml_label(std::string_view str);

ret_t to_xml_attr_1(std::string_view str);
ret_t to_xml_attr_2(std::string_view str);

ret_t to_xml_text(std::string_view str);
ret_t to_xml_cdata(std::string_view str);
ret_t to_xml_comment(std::string_view str);
ret_t to_xml_proc(std::string_view str);

std::string_view unescape_xml(std::string_view sv); //It should be a span. String views are assumed immutable.

}
}