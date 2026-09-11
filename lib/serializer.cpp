#include <charconv>
#include <stdexcept>
#include <vs-xml/commons.hpp>
#include <vs-xml/serializer.hpp>

namespace VS_XML_NS{
namespace serialize{

//TODO: Add support to output &#... escapes. Added in the new functions, but they still need replacing in current code.

std::string_view validate_xml_label(std::string_view str, bool optional) noexcept(VS_XML_NO_EXCEPT){
#if !VS_XML_NO_EXCEPT
    if(!is_valid_xml_label(str, optional)){
        throw std::runtime_error("Invalid XML label");
    }
#endif
    return str;
}

bool is_valid_xml_label(std::string_view str, bool optional) noexcept{
    if(str.empty())return optional;

    if (str[0]=='_' or (str[0]>='a' && str[0]<='z') or (str[0]>='A' && str[0]<='Z')){}
    else return false;

    for(size_t i=1;i<str.size();i++){
        char c = str[i];
        //In theory some intervals of utf8 should be negated. But this filter is good enough for now.
        if(c=='_' or c=='.' or c=='-' or (c>='0' && c<='9') or (c>='a' && c<='z') or (c>='A' && c<='Z') or (c>127)){/*OK*/}
        else return false;
    }
    return true;
}

ret_t to_xml_attr_1(std::string_view str){
    //Escaping for single-quoted attributes.
    size_t extra = 0;
    for(char c : str){
        if(c=='<')extra += 3;
        else if(c=='&')extra += 4;
        else if(c=='\'')extra += 3;
    }
    if(extra==0)return str;

    std::string tmp;
    tmp.reserve(str.size()+extra);
    for(char c : str){
        if(c=='<')tmp+="&lt;";
        else if(c=='&')tmp+="&amp;";
        else if(c=='\'')tmp+="&apos;";
        else tmp+=c;
    }
    return tmp;
}

ret_t to_xml_attr_2(std::string_view str){
    int rule_a = 0;
    for(auto& c : str){
        if(c=='<')rule_a+=sizeof("&lt;")-1-1;
        else if(c=='&')rule_a+=sizeof("&amp;")-1-1;
        else if(c=='"')rule_a+=sizeof("&quot;")-1-1;
        //Maybe some config to enable the others as well even if not needed for a correct XML serialization?
    }
    if(rule_a==0)return str;
    else{
        std::string tmp;
        tmp.reserve(str.length()+rule_a);
        for(auto& c : str){
            if(c=='<')tmp+="&lt;";
            else if(c=='&')tmp+="&amp;";
            else if(c=='"')tmp+="&quot;";
            else tmp+=c;
        }
        return tmp;
    }
}

ret_t to_xml_text(std::string_view str){
    // `]]>` is not allowed in text and must be written as `]]&gt;`.
    size_t extra = 0;
    for(size_t i=0;i<str.size();i++){
        if(str[i]=='<')extra += 3;
        else if(str[i]=='&')extra += 4;
        else if(str[i]==']' && i+2<str.size() && str[i+1]==']' && str[i+2]=='>')extra += 3;
    }
    if(extra==0)return str;

    std::string tmp;
    tmp.reserve(str.size()+extra);
    for(size_t i=0;i<str.size();i++){
        char c = str[i];
        if(c=='<')tmp+="&lt;";
        else if(c=='&')tmp+="&amp;";
        else if(c==']' && i+2<str.size() && str[i+1]==']' && str[i+2]=='>'){
            tmp+="]]&gt;";
            i+=2; // consume the whole `]]>`
        }
        else tmp+=c;
    }
    return tmp;
}

ret_t to_xml_cdata(std::string_view str){
    int rule_a = 0;
    for(auto& c : str){
        if(rule_a==0 && c==']')rule_a=1;
        else if(rule_a==1 && c==']')rule_a=2;
        else if(rule_a==2 && c=='>')return {};   //Disallowed sequence ]]>
        else rule_a=0;
    }
    return str;
}

ret_t to_xml_comment(std::string_view str){
    int rule_a = 0;
    for(auto& c : str){
        if(rule_a==0 && c=='-')rule_a=1;
        else if(rule_a==1 && c=='-')return {};   //Disallowed sequence --
        else rule_a=0;
    }
    return str;
}

ret_t to_xml_proc(std::string_view str){
    int rule_a = 0;
    for(auto& c : str){
        if(rule_a==0 && c=='?')rule_a=1;
        else if(rule_a==1 && c=='>')return {};   //Disallowed sequence ?>
        else rule_a=0;
    }
    return str;
}


std::string escape_xml(std::string_view sv){
    std::string out;
    out.reserve(sv.size());
    for(char c : sv){
        if(c=='&')out+="&amp;";
        else if(c=='<')out+="&lt;";
        else if(c=='>')out+="&gt;";
        else if(c=='\'')out+="&apos;";
        else if(c=='"')out+="&quot;";
        else out+=c;
    }
    return out;
}

std::string_view inplace_unescape_xml(std::string_view sv) {
    //It should be a span. String views are assumed immutable.
    //We assume that sv.data() points to mutable memory.
    char *buffer = const_cast<char*>(sv.data());
    size_t len = sv.size();
    size_t read = 0;
    size_t write = 0;

    while (read < len) {
        if (buffer[read] == '&') {
            // Check known entities.
            if (read + 3 < len && std::string_view(buffer + read, 4) == "&lt;") {
                buffer[write++] = '<';
                read += 4;
            } else if (read + 3 < len && std::string_view(buffer + read, 4) == "&gt;") {
                buffer[write++] = '>';
                read += 4;
            } else if (read + 4 < len && std::string_view(buffer + read, 5) == "&amp;") {
                buffer[write++] = '&';
                read += 5;
            } else if (read + 5 < len && std::string_view(buffer + read, 6) == "&quot;") {
                buffer[write++] = '\"';
                read += 6;
            } else if (read + 5 < len && std::string_view(buffer + read, 6) == "&apos;") {
                buffer[write++] = '\'';
                read += 6;
            } else if (read + 1 < len && buffer[read + 1] == '#') {
                // Numeric entity.
                size_t j = read + 2;
                bool hex = false;
                if (j < len && (buffer[j] == 'x' || buffer[j] == 'X')) {
                    hex = true;
                    ++j;
                }
                size_t numStart = j;
                while (j < len && buffer[j] != ';')
                    ++j;
                if (j < len && buffer[j] == ';') {
                    // Convert to an integer, keeping the reference literal if it is
                    // invalid or outside the single-byte range we can represent.
                    unsigned long code = 0;
                    auto [ptr, ec] = std::from_chars(buffer+numStart, buffer+j, code, hex ? 16 : 10);
                    if (ec == std::errc{} && code >= 1 && code <= 0xFF) {
                        buffer[write++] = static_cast<char>(code);
                        read = j + 1;
                    } else {
                        buffer[write++] = buffer[read++];
                    }
                } else {
                    // No semicolon found; treat as literal.
                    buffer[write++] = buffer[read++];
                }
            } else {
                // Unknown entity; copy '&'
                buffer[write++] = buffer[read++];
            }
        } else {
            buffer[write++] = buffer[read++];
        }
    }
    
    return std::string_view(buffer, write);
}

//////////////

}}