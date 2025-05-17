#include <iostream>
#include <vs-xml/serializer.hpp>

// ------------------------------------------------------------
// Demonstration of usage
// ------------------------------------------------------------
int main() {
    // Test string with characters that need escaping.
    std::string original = "A&B <C>'D\"E >";
    // EscapedView: will expand special chars into their corresponding XML escapes.
    xml::serialize::escaped_view escaped(original);
    std::string escapedResult;
    for (char c : escaped)
        escapedResult.push_back(c);
    std::cout << "Escaped version of \"" << original << "\":\n" << escapedResult << "\n";

    // Now create an XML-escaped version manually including numeric escapes.
    // For example, we can have named escapes and numeric escapes:
    std::string xmlEscaped = "A&amp;B &lt;C&gt;&apos;D&quot;E &gt; &#65; &#x42;";
    // The numeric escapes "&#65;" and "&#x42;" should resolve to 'A' and 'B', respectively.

    xml::serialize::unescaped_view  unescaped(xmlEscaped);
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
    for (char c : xml::serialize::escaped_view (unescapedResult))
        reescaped.push_back(c);
    std::cout << "Re-escaped version of unescaped result:\n" << reescaped << "\n";

    return 0;
}