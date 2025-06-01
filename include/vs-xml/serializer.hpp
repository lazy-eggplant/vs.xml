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

#include <charconv>
#include <expected>

#include <optional>
#include <variant>
#include <string>
#include <string_view>
#include <array>

#include <vs-xml/commons.hpp>


namespace VS_XML_NS{
namespace serialize{
//https://stackoverflow.com/questions/1091945/what-characters-do-i-need-to-escape-in-xml-documents
//An attempt will be made to keep the input string_view, unless escaping is needed.

typedef std::optional<std::variant<std::string,std::string_view>> ret_t;

std::string_view validate_xml_label(std::string_view str, bool optional=false);

ret_t to_xml_attr_1(std::string_view str);
ret_t to_xml_attr_2(std::string_view str);

ret_t to_xml_text(std::string_view str);
ret_t to_xml_cdata(std::string_view str);
ret_t to_xml_comment(std::string_view str);
ret_t to_xml_proc(std::string_view str);

std::string_view inplace_unescape_xml(std::string_view sv); //It should be a span. String views are assumed immutable.
constexpr std::string escape_xml(std::string_view sv); //TODO: Implement

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//TODO: move impl out in the C file to keep this reasonable.

// ------------------------------------------------------------
// Utility functions and structures for XML escapes
// ------------------------------------------------------------

struct xml_escape_seq_t {
    char ch;                        // character to be escaped/unescaped
    std::string_view escape;        // corresponding escape sequence, e.g. "&amp;"
};

constexpr std::array<xml_escape_seq_t, 5> entities{{
    { '&', "&amp;"  },
    { '<', "&lt;"   },
    { '>', "&gt;"   },
    { '\'', "&apos;" },
    { '"', "&quot;" }
}};

// Given a character, returns the corresponding escape string if it needs escaping,
// otherwise returns an empty string.
inline std::string_view entities_map(char c) {
    for (const auto& xml : entities) {
        if (xml.ch == c)
            return xml.escape;
    }
    return {};
}

/**
 * @brief A weak container allowing to iterate characters from an escaped string unescaping them
 * 
 */
class escaped_view {
public:
    explicit escaped_view(std::string_view sv) : sv_(sv) {}

    class iterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type        = char;
        using difference_type   = std::ptrdiff_t;
        using pointer           = const char*;
        using reference         = char;

        iterator() : parent_(nullptr), pos_(0), escapePos_(0) {}
        iterator(const escaped_view* parent, size_t pos)
            : parent_(parent), pos_(pos), escapePos_(0) {}

        char operator*() const noexcept(VS_XML_NO_EXCEPT){
            if (parent_ == nullptr || pos_ >= parent_->sv_.size()) {
                xml_assert(true,"Dereferencing end iterator");
                //throw std::out_of_range("Dereferencing end iterator");
            }
            // When expanding an escape sequence, return the proper character.
            if (inEscapeExpansion())
                return currentEscape()[escapePos_];
            else {
                char c = parent_->sv_[pos_];
                std::string_view esc = entities_map(c);
                if (!esc.empty())
                    return esc.front();
                else
                    return c;
            }
        }

        iterator& operator++() {
            if (parent_ == nullptr || pos_ > parent_->sv_.size())
                return *this;

            if (inEscapeExpansion()) {
                ++escapePos_;
                if (escapePos_ >= currentEscape().size()) {
                    escapePos_ = 0;
                    ++pos_;
                }
            } else {
                char c = parent_->sv_[pos_];
                std::string_view esc = entities_map(c);
                if (!esc.empty())
                    escapePos_ = 1;
                else
                    ++pos_;
            }
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        bool operator==(const iterator &other) const {
            return parent_ == other.parent_ &&
                   pos_ == other.pos_ &&
                   escapePos_ == other.escapePos_;
        }
        bool operator!=(const iterator &other) const {
            return !(*this == other);
        }
    private:
        bool inEscapeExpansion() const {
            if (parent_ == nullptr || pos_ >= parent_->sv_.size())
                return false;
            char c = parent_->sv_[pos_];
            std::string_view esc = entities_map(c);
            return !esc.empty() && escapePos_ > 0;
        }
        std::string_view currentEscape() const {
            char c = parent_->sv_[pos_];
            return entities_map(c);
        }

        const escaped_view* parent_;
        size_t pos_;
        size_t escapePos_;
    };

    iterator begin() const { return iterator(this, 0); }
    iterator end() const { return iterator(this, sv_.size()); }
private:
    std::string_view sv_;
};

static_assert(std::input_iterator<escaped_view::iterator>);

/**
 * @brief A weak container allowing to iterate characters from an unescaped string escaping them
 * 
 */
class unescaped_view {
public:
    explicit unescaped_view(std::string_view sv) : sv_(sv) {}

    class iterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type        = char;
        using difference_type   = std::ptrdiff_t;
        using pointer           = const char*;
        using reference         = char;

        iterator() : parent_(nullptr), pos_(0) {}
        iterator(const unescaped_view* parent, size_t pos)
            : parent_(parent), pos_(pos) {}

        char operator*() const noexcept(VS_XML_NO_EXCEPT){
            if (parent_ == nullptr || pos_ >= parent_->sv_.size())
                xml_assert(true,"Dereferencing end iterator");
                //throw std::out_of_range("Dereferencing end iterator");

            if (auto [ch, len] = detectEscape(parent_->sv_.substr(pos_)); len > 0)
                return ch;
            else
                return parent_->sv_[pos_];
        }

        iterator& operator++() {
            if (parent_ == nullptr || pos_ > parent_->sv_.size())
                return *this;
            if (auto [ch, len] = detectEscape(parent_->sv_.substr(pos_)); len > 0)
                pos_ += len;
            else
                ++pos_;
            return *this;
        }
        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        bool operator==(const iterator &other) const {
            return parent_ == other.parent_ && pos_ == other.pos_;
        }
        bool operator!=(const iterator &other) const {
            return !(*this == other);
        }
    private:
        // Returns {unescaped character, length consumed} if an escape is detected.
        static std::pair<char, size_t> detectEscape(std::string_view sv) {
            // Numeric escape first.
            if (sv.size() > 2 && sv[1] == '#') {
                if (auto [ch, len] = detectNumericEscape(sv); len > 0)
                    return { ch, len };
            }
            // Then named escapes.
            for (const auto& xml : entities) {
                std::string_view esc(xml.escape);
                if (sv.substr(0, esc.size()) == esc)
                    return { xml.ch, esc.size() };
            }
            return { 0, 0 };
        }
        // Handles numeric escapes: both hexadecimal (&#xhhhh;) and decimal (&#nnnn;).
        static std::pair<char, size_t> detectNumericEscape(std::string_view sv) {
            if (sv.size() < 4) // minimal: "&#0;"
                return { 0, 0 };
            if (sv[0] != '&' || sv[1] != '#')
                return { 0, 0 };

            size_t pos = 2;
            bool isHex = false;
            if (pos < sv.size() && (sv[pos] == 'x' || sv[pos] == 'X')) {
                isHex = true;
                pos++;
            }
            if (pos >= sv.size() || !std::isxdigit(static_cast<unsigned char>(sv[pos])))
                return { 0, 0 };

            size_t startDigits = pos;
            while (pos < sv.size() && std::isxdigit(static_cast<unsigned char>(sv[pos])))
                pos++;
            if (pos >= sv.size() || sv[pos] != ';')
                return { 0, 0 };
            std::string numberStr(sv.substr(startDigits, pos - startDigits));
            unsigned long value = 0;
            auto [ptr, ec] = std::from_chars(numberStr.data(), numberStr.data()+ numberStr.size(), value, isHex ? 16 : 10);
            if (ec != std::errc{}) {
                return { 0, 0 };
            }
            if (value > 0xFF)
                return { 0, 0 };
            return { static_cast<char>(value), pos + 1 };
        }
        const unescaped_view* parent_;
        size_t pos_;
    };

    iterator begin() const { return iterator(this, 0); }
    iterator end() const { return iterator(this, sv_.size()); }
private:
    std::string_view sv_;
};

static_assert(std::input_iterator<unescaped_view::iterator>);


}
}


