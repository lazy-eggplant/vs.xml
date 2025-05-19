#pragma once

/**
 * @file parser.hpp
 * @author karurochari
 * @brief Implementation of the parser logic
 * @date 2025-05-04
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <span>

#include "commons.hpp"
#include "serializer.hpp"

namespace VS_XML_NS{


/**
 * @brief Generic interface for the XML parser, responsible for filling in a builder
 * 
 * @tparam Builder_t the builder class on which this parser is being based.
 */
template<ProperBuilder Builder_t>
class Parser {
public:
    Parser(std::span<char> data, Builder_t &builder)
        : data_(data.data(),data.size()), pos_(0), builder_(builder)
    {}

    Parser(std::string_view data, Builder_t &builder)
        : data_(data.data(),data.size()), pos_(0), builder_(builder)
    {
        static_assert(Builder_t::configs.raw_strings, "Cannot pass immutable buffer if the input strings might need mutation");
    }

    struct error_t{
        enum ErrorCode {
            OK = 0,
            MISSING_LT_BEGIN,       // "Expected '<' at beginning of XML document."
            UNTERMINATED_PROC,      // "Unterminated processing instruction."
            UNTERMINATED_COMMENT,   // "Unterminated XML comment."
            UNTERMINATED_CDATA,     // "Unterminated CDATA section."
            UNTERMINATED_ATTR_VALUE,// "Unterminated attribute value."
            MISSING_ATTR_EQUALS,    // "Expected '=' after attribute name."
            MISSING_ATTR_QUOTES,    // "Expected quote for attribute value."
            MISSING_GT_AFTER_TAG,   // "Expected '>' after tag name and attributes."
            MISSING_GT_IN_END_TAG,  // "Expected '>' in closing tag."
            UNEXPECTED_EOF,         // "Unexpected end of XML content."
            NODE_NOT_ALLOWED_ROOT   // "Node type not allowed in the document root."
        } code;
        size_t ctx; // current position in the data
        
        std::string_view msg() const {
            switch (code) {
                case OK:                        return "OK";
                case MISSING_LT_BEGIN:          return "Expected '<' at beginning of XML document.";
                case UNTERMINATED_PROC:         return "Unterminated processing instruction.";
                case UNTERMINATED_COMMENT:      return "Unterminated XML comment.";
                case UNTERMINATED_CDATA:        return "Unterminated CDATA section.";
                case UNTERMINATED_ATTR_VALUE:   return "Unterminated attribute value.";
                case MISSING_ATTR_EQUALS:       return "Expected '=' after attribute name.";
                case MISSING_ATTR_QUOTES:       return "Expected quote for attribute value.";
                case MISSING_GT_AFTER_TAG:      return "Expected '>' after tag name and attributes.";
                case MISSING_GT_IN_END_TAG:     return "Expected '>' in closing tag.";
                case UNEXPECTED_EOF:            return "Unexpected end of XML content.";
                case NODE_NOT_ALLOWED_ROOT:     return "Node type not allowed in the document root.";
                default:                        return "Unknown error.";
            }
        }
    };

    // Start parsing from the beginning of the XML document.
    // Throws an exception on error.
    [[nodiscard("Don't discard the return value for parsing!")]] std::expected<void, error_t> parse() noexcept{
        if constexpr(!Builder_t::is_document) {
            skip_whitespace();
            // Expecting the first tag to begin with '<'
            if (!consume('<'))
                return std::unexpected(error_t{error_t::MISSING_LT_BEGIN, pos_});
            parseElement<Builder_t::is_document>();
        }
        else {
            while (pos_ < data_.size()) {
                skip_whitespace();
                if (!consume('<'))
                    return std::unexpected(error_t{error_t::MISSING_LT_BEGIN, pos_});
                parseElement<Builder_t::is_document>();
            }
        }
        return {};
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
    template<bool ROOT=false>
    std::expected<void, error_t> parseElement() noexcept{
        skip_whitespace();
        if (pos_ >= data_.size())
            return {};

        // Check for processing instruction <? ... ?>
        if (peek('?')) {
            ++pos_;  // skip '?'
            // Read processing instruction until "?>"
            size_t procEnd = data_.find("?>", pos_);
            if (procEnd == std::string_view::npos)
                { return std::unexpected(error_t{error_t::UNTERMINATED_PROC, pos_}); }
            auto procContent = data_.substr(pos_, procEnd - pos_);
            if constexpr (Builder_t::configs.raw_strings ) builder_.proc(procContent);
            else builder_.proc(serialize::inplace_unescape_xml(procContent));
            pos_ = procEnd + 2;
            return {};
        }

        // Check for comment or CDATA (both start with "!")
        if (peek('!')) {
            ++pos_;  // skip '!'
            // Comment: <!-- ... -->
            if (data_.substr(pos_, 2) == "--") {
                pos_ += 2;
                size_t commentEnd = data_.find("-->", pos_);
                if (commentEnd == std::string_view::npos)
                    { return std::unexpected(error_t{error_t::UNTERMINATED_COMMENT, pos_}); }
                auto commentContent = data_.substr(pos_, commentEnd - pos_);
                if constexpr (Builder_t::configs.raw_strings ) builder_.comment(commentContent);
                else builder_.comment(serialize::inplace_unescape_xml(commentContent));
                pos_ = commentEnd + 3;
                return {};
            } 
            // CDATA: <![CDATA[ ... ]]>
            else if (data_.substr(pos_, 7) == "[CDATA[" && !ROOT) {
                pos_ += 7; // skip "[CDATA["
                size_t cdataEnd = data_.find("]]>", pos_);
                if (cdataEnd == std::string_view::npos)
                    { return std::unexpected(error_t{error_t::UNTERMINATED_CDATA, pos_}); }
                auto cdataContent = data_.substr(pos_, cdataEnd - pos_);
                builder_.cdata(cdataContent); // CDATA content provided as-is.
                pos_ = cdataEnd + 3;
                return {};
            } 
            else if(!ROOT){
                // Other declarations - skip until '>'
                get_until('>');
                consume('>');
                return {};
            } 
            else return std::unexpected(error_t{error_t::NODE_NOT_ALLOWED_ROOT, pos_});
      
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
            if (!consume('='))return std::unexpected(error_t{error_t::MISSING_ATTR_EQUALS, pos_}); 
            skip_whitespace();
            // Expect a quoted attribute value.
            char quote = 0;
            if (peek('\"') || peek('\'')) {
                quote = data_[pos_];
                ++pos_;
            } 
            else return std::unexpected(error_t{error_t::MISSING_ATTR_QUOTES, pos_}); 
            

            size_t attrValueStart = pos_;
            while (pos_ < data_.size() && data_[pos_] != quote) ++pos_;
            if (pos_ >= data_.size()) return std::unexpected(error_t{error_t::UNTERMINATED_ATTR_VALUE, pos_}); 
            auto attrValue = data_.substr(attrValueStart, pos_ - attrValueStart);
            if (!consume(quote))return std::unexpected(error_t{error_t::MISSING_ATTR_QUOTES, pos_}); 

            // Call builder's attr with local name and corresponding namespace.
            if constexpr (Builder_t::configs.raw_strings ) builder_.attr(attrLocal, attrValue, attrNs);
            else builder_.attr(attrLocal, serialize::inplace_unescape_xml(attrValue), attrNs);
            
        }

        // Self-closing element?
        if (data_.substr(pos_, 2) == "/>") {
            pos_ += 2;
            builder_.end();
            return {};
        }

        // Otherwise, consume the '>' and open the element.
        if (!consume('>')) return std::unexpected(error_t{error_t::MISSING_GT_AFTER_TAG, pos_}); 

        // Parse content within the element until encountering a matching end tag.
        while (pos_ < data_.size()) {
            skip_whitespace();
            if (pos_ >= data_.size()) return std::unexpected(error_t{error_t::UNEXPECTED_EOF, pos_}); 

            if (peek('<')) {
                // Check for closing tag.
                if (data_.substr(pos_, 2) == "</") {
                    pos_ += 2; // skip "</"
                    // Skip the qualified name inside end tag.
                    get_until('>');
                    if (!consume('>')) return std::unexpected(error_t{error_t::MISSING_GT_IN_END_TAG, pos_});
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
                else unescapedText = serialize::inplace_unescape_xml(textContent);

                if (!unescapedText.empty() &&
                    unescapedText.find_first_not_of(" \t\r\n") != std::string::npos)
                {
                    builder_.text(unescapedText);
                }
            }
        }

        return {};
    }
};


}