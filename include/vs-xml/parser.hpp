#pragma once


/**
 * @file parser.hpp
 * @author karurochari
 * @brief Implementation of the parse logic
 * @date 2025-05-04
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <span>
#include "commons.hpp"
#include "serializer.hpp"

namespace VS_XML_NS{


//-----------------------------------------------------
// XML Parser class
//-----------------------------------------------------
template<ProperBuilder Builder_t>
class Parser {
public:
    Parser(std::span<char> data, Builder_t &builder)
        : data_(data.data(),data.size()), pos_(0), builder_(builder)
    {}

    // Start parsing from the beginning of the XML document.
    // Throws an exception on error.
    void parse() {
        skip_whitespace();
        // Expecting the first tag to begin with '<'
        if (!consume('<'))
            throw std::runtime_error("Expected '<' at beginning of XML document.");
            
        parseElement();
    }

private:
    //This should logically be a span<char>, but we don't have *.find for it, so we keep it as is for now.
    std::string_view data_;
    size_t pos_;
    Builder_t &builder_;

    //-----------------------------------------------------
    // Helper: Split a qualified name "prefix:local" into namespace and local name.
    // If no colon exists, returns { name, "" }.
    // The returned pair is in the form (local name, namespace)
    // because builder calls are of the form begin(localName, ns).
    static std::pair<std::string_view, std::string_view> split_namespace(std::string_view qualified) {
        size_t pos = qualified.find(':');
        if (pos != std::string_view::npos) {
            // Namespace is the left side, local name is right side.
            return { qualified.substr(pos + 1), qualified.substr(0, pos) };
        }
        return { qualified, "" };
    }


    // Skip whitespace characters.
    void skip_whitespace() {
        while (pos_ < data_.size() && std::isspace(static_cast<unsigned char>(data_[pos_])))
            ++pos_;
    }

    // Consume a single character if it matches ch.
    bool consume(char ch) {
        if (pos_ < data_.size() && data_[pos_] == ch) {
            ++pos_;
            return true;
        }
        return false;
    }

    // Peek to see if the current character equals ch.
    bool peek(char ch) {
        return (pos_ < data_.size() && data_[pos_] == ch);
    }

    // Read until a given delimiter; does not consume the delimiter.
    std::string_view get_until(char delim) {
        size_t start = pos_;
        while (pos_ < data_.size() && data_[pos_] != delim)
            ++pos_;
        return data_.substr(start, pos_ - start);
    }

    // Read while predicate returns true.
    std::string_view get_while(bool (*pred)(char)) {
        size_t start = pos_;
        while (pos_ < data_.size() && pred(data_[pos_]))
            ++pos_;
        return data_.substr(start, pos_ - start);
    }

    // Main element parser. It assumes that a '<' has already been consumed.
    void parseElement() {
        skip_whitespace();
        if (pos_ >= data_.size())
            return;

        // Check for processing instruction <? ... ?>
        if (peek('?')) {
            ++pos_;  // skip '?'
            // Read processing instruction until "?>"
            size_t procEnd = data_.find("?>", pos_);
            if (procEnd == std::string_view::npos)
                throw std::runtime_error("Unterminated processing instruction.");
            auto procContent = data_.substr(pos_, procEnd - pos_);
            if constexpr (Builder_t::configs.raw_strings ) builder_.proc(procContent);
            else builder_.proc(serialize::unescape_xml(procContent));
            pos_ = procEnd + 2;
            return;
        }

        // Check for comment or CDATA (both start with "!")
        if (peek('!')) {
            ++pos_;  // skip '!'
            // Comment: <!-- ... -->
            if (data_.substr(pos_, 2) == "--") {
                pos_ += 2;
                size_t commentEnd = data_.find("-->", pos_);
                if (commentEnd == std::string_view::npos)
                    throw std::runtime_error("Unterminated XML comment.");
                auto commentContent = data_.substr(pos_, commentEnd - pos_);
                if constexpr (Builder_t::configs.raw_strings ) builder_.comment(commentContent);
                else builder_.comment(serialize::unescape_xml(commentContent));
                pos_ = commentEnd + 3;
                return;
            } 
            // CDATA: <![CDATA[ ... ]]>
            else if (data_.substr(pos_, 7) == "[CDATA[") {
                pos_ += 7; // skip "[CDATA["
                size_t cdataEnd = data_.find("]]>", pos_);
                if (cdataEnd == std::string_view::npos)
                    throw std::runtime_error("Unterminated CDATA section.");
                auto cdataContent = data_.substr(pos_, cdataEnd - pos_);
                builder_.cdata(cdataContent); // CDATA content provided as-is.
                pos_ = cdataEnd + 3;
                return;
            } else {
                // Other declarations - skip until '>'
                get_until('>');
                consume('>');
                return;
            }
        }

        // Standard element:
        // Parse element qualified name. Allowed characters: alphanumeric, '_', ':', '-'
        auto qualifiedName = get_while([](char c) {
            return std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == ':' || c == '-';
        });
        auto [localName, ns] = split_namespace(qualifiedName);
        
        builder_.begin(localName, ns);

        // Parse attributes (if any)
        while (true) {
            skip_whitespace();
            // End of tag? Either '>' or '/>'
            if (data_.substr(pos_, 2) == "/>" || peek('>'))
                break;

            // Parse attribute qualified name.
            auto attrQualified = get_while([](char c) {
                return std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == ':' || c == '-';
            });
            auto [attrLocal, attrNs] = split_namespace(attrQualified);
            
            skip_whitespace();
            if (!consume('='))
                throw std::runtime_error("Expected '=' after attribute name.");
            skip_whitespace();
            // Expect a quoted attribute value.
            char quote = 0;
            if (peek('\"') || peek('\'')) {
                quote = data_[pos_];
                ++pos_;
            } else {
                throw std::runtime_error("Expected quote for attribute value.");
            }

            size_t attrValueStart = pos_;
            while (pos_ < data_.size() && data_[pos_] != quote)
                ++pos_;
            if (pos_ >= data_.size())
                throw std::runtime_error("Unterminated attribute value.");
            auto attrValue = data_.substr(attrValueStart, pos_ - attrValueStart);
            if (!consume(quote))
                throw std::runtime_error("Expected closing quote for attribute value.");

            // Call builder's attr with local name and corresponding namespace.
            if constexpr (Builder_t::configs.raw_strings ) builder_.attr(attrLocal, attrValue, attrNs);
            else builder_.attr(attrLocal, serialize::unescape_xml(attrValue), attrNs);
            
        }

        // Self-closing element?
        if (data_.substr(pos_, 2) == "/>") {
            pos_ += 2;
            builder_.end();
            return;
        }

        // Otherwise, consume the '>' and open the element.
        if (!consume('>'))
            throw std::runtime_error("Expected '>' after tag name and attributes.");

        // Parse content within the element until encountering a matching end tag.
        while (pos_ < data_.size()) {
            skip_whitespace();
            if (pos_ >= data_.size())
                throw std::runtime_error("Unexpected end of XML content.");

            if (peek('<')) {
                // Check for closing tag.
                if (data_.substr(pos_, 2) == "</") {
                    pos_ += 2; // skip "</"
                    // Skip the qualified name inside end tag.
                    get_until('>');
                    if (!consume('>'))
                        throw std::runtime_error("Expected '>' in closing tag.");
                    builder_.end();
                    break;
                } else {
                    // Child element or special node.
                    ++pos_; // skip '<'
                    parseElement(); // recursive call
                }
            } else {
                // Process text content until next '<'
                size_t textStart = pos_;
                while (pos_ < data_.size() && data_[pos_] != '<')
                    ++pos_;
                auto textContent = data_.substr(textStart, pos_ - textStart);
                std::string_view unescapedText;
                
                if constexpr (Builder_t::configs.raw_strings )unescapedText=textContent;
                else unescapedText = serialize::unescape_xml(textContent);

                if (!unescapedText.empty() &&
                    unescapedText.find_first_not_of(" \t\r\n") != std::string::npos)
                {
                    builder_.text(unescapedText);
                }
            }
        }
    }
}; // end class XmlParser


}