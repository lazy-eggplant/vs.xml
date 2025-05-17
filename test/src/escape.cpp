#include <string_view>
#include <string>
#include <iterator>
#include <stdexcept>
#include <cassert>
#include <iostream>
#include <array>
#include <cctype>
#include <cstdlib>

// ------------------------------------------------------------
// Utility functions and structures for XML escapes
// ------------------------------------------------------------

struct XmlEscape {
    char ch;                        // character to be escaped/unescaped
    std::string_view escape;        // corresponding escape sequence, e.g. "&amp;"
};

constexpr std::array<XmlEscape, 5> xmlEscapes{{
    { '&', "&amp;"  },
    { '<', "&lt;"   },
    { '>', "&gt;"   },
    { '\'', "&apos;" },
    { '"', "&quot;" }
}};

// Given a character, returns the corresponding escape string if it needs escaping,
// otherwise returns an empty string.
std::string_view getEscapeForChar(char c) {
    for (const auto& xml : xmlEscapes) {
        if (xml.ch == c)
            return xml.escape;
    }
    return {};
}

// ------------------------------------------------------------
// EscapedView: Escapes all six special characters (with numeric escapes not produced)
// ------------------------------------------------------------
class EscapedView {
public:
    explicit EscapedView(std::string_view sv) : sv_(sv) {}

    class iterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type        = char;
        using difference_type   = std::ptrdiff_t;
        using pointer           = const char*;
        using reference         = char;

        iterator() : parent_(nullptr), pos_(0), escapePos_(0) {}
        iterator(const EscapedView* parent, size_t pos)
            : parent_(parent), pos_(pos), escapePos_(0) {}

        char operator*() const {
            if (parent_ == nullptr || pos_ >= parent_->sv_.size()) {
                throw std::out_of_range("Dereferencing end iterator");
            }
            // When expanding an escape sequence, return the proper character.
            if (inEscapeExpansion())
                return currentEscape().at(escapePos_);
            else {
                char c = parent_->sv_[pos_];
                std::string_view esc = getEscapeForChar(c);
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
                std::string_view esc = getEscapeForChar(c);
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
            std::string_view esc = getEscapeForChar(c);
            return !esc.empty() && escapePos_ > 0;
        }
        std::string_view currentEscape() const {
            char c = parent_->sv_[pos_];
            return getEscapeForChar(c);
        }

        const EscapedView* parent_;
        size_t pos_;
        size_t escapePos_;
    };

    iterator begin() const { return iterator(this, 0); }
    iterator end() const { return iterator(this, sv_.size()); }
private:
    std::string_view sv_;
};

static_assert(std::input_iterator<EscapedView::iterator>);

// ------------------------------------------------------------
// UnescapedView: Unescapes both the five named entities and numeric escapes.
// ------------------------------------------------------------
class UnescapedView {
public:
    explicit UnescapedView(std::string_view sv) : sv_(sv) {}

    class iterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type        = char;
        using difference_type   = std::ptrdiff_t;
        using pointer           = const char*;
        using reference         = char;

        iterator() : parent_(nullptr), pos_(0) {}
        iterator(const UnescapedView* parent, size_t pos)
            : parent_(parent), pos_(pos) {}

        char operator*() const {
            if (parent_ == nullptr || pos_ >= parent_->sv_.size())
                throw std::out_of_range("Dereferencing end iterator");

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
            for (const auto& xml : xmlEscapes) {
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
            try {
                value = std::stoul(numberStr, nullptr, isHex ? 16 : 10);
            } catch (...) {
                return { 0, 0 };
            }
            if (value > 0xFF)
                return { 0, 0 };
            return { static_cast<char>(value), pos + 1 };
        }
        const UnescapedView* parent_;
        size_t pos_;
    };

    iterator begin() const { return iterator(this, 0); }
    iterator end() const { return iterator(this, sv_.size()); }
private:
    std::string_view sv_;
};

static_assert(std::input_iterator<UnescapedView::iterator>);





// ------------------------------------------------------------
// Demonstration of usage
// ------------------------------------------------------------
int main() {
    // Test string with characters that need escaping.
    std::string original = "A&B <C>'D\"E >";
    // EscapedView: will expand special chars into their corresponding XML escapes.
    EscapedView escaped(original);
    std::string escapedResult;
    for (char c : escaped)
        escapedResult.push_back(c);
    std::cout << "Escaped version of \"" << original << "\":\n" << escapedResult << "\n";

    // Now create an XML-escaped version manually including numeric escapes.
    // For example, we can have named escapes and numeric escapes:
    std::string xmlEscaped = "A&amp;B &lt;C&gt;&apos;D&quot;E &gt; &#65; &#x42;";
    // The numeric escapes "&#65;" and "&#x42;" should resolve to 'A' and 'B', respectively.

    UnescapedView unescaped(xmlEscaped);
    std::string unescapedResult;
    for (char c : unescaped)
        unescapedResult.push_back(c);
    std::cout << "Unescaped version of \"" << xmlEscaped << "\":\n" << unescapedResult << "\n";

    // Expected unescaped version:
    // "A&B <C>'D\"E > A B"  (with a space before the numeric escapes)
    // Note: In the above string, after "E >" there is a space then "&#65;" => 'A', then a space,
    // then "&#x42;" => 'B'.
    std::string expectedUnescaped = "A&B <C>'D\"E > A B";
    assert(unescapedResult == expectedUnescaped);

    // Also, if we escape the unescaped version (which produces only the five named escapes),
    // we get back a limited form.
    std::string reescaped;
    for (char c : EscapedView(unescapedResult))
        reescaped.push_back(c);
    std::cout << "Re-escaped version of unescaped result:\n" << reescaped << "\n";

    return 0;
}