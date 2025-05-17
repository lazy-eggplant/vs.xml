#include <charconv>
#include <stdexcept>
#include <vs-xml/commons.hpp>
#include <vs-xml/serializer.hpp>

namespace VS_XML_NS{
namespace serialize{

//TODO: Add support to output &#... escapes

std::string_view validate_xml_label(std::string_view str){
    //This operation alone is responsible for around 10% lower speed while parsing. It might be good to disable it if not needed?
    if(str.size()==0)return str;
    
    if (str[0]=='_' or (str[0]>='a' && str[0]<='z') or (str[0]>='A' && str[0]<='Z')){}
    else [[unlikely]] throw std::runtime_error("Invalid XML label");
    
    for(auto& c : std::string_view{str.begin()+1,str.end()}){
        //In theory some intervals of utf8 should be negated. But this filter is good enough for now.
        if((c=='_' or c=='.' or c=='-' or (c>='0' && c<='9') or (c>='a' && c<='z') or (c>='A' && c<='Z') or (c>127))){/*OK*/}
        else [[unlikely]] {
            throw std::runtime_error("Invalid XML label");
        }
    }
    return str;
}

ret_t to_xml_attr_1(std::string_view str){
    int rule_a = 0;
    for(auto& c : str){
        if(c=='<')rule_a+=sizeof("&lt;")-1-1;
        else if(c=='&')rule_a+=sizeof("&amp;")-1-1;
        else if(c=='\'')rule_a+=sizeof("&apos;")-1-1;
        //Maybe some config to enable the others as well even if not needed for a correct XML serialization?
    }
    if(rule_a==0)return str;
    else{
        std::string tmp;
        tmp.reserve(str.length()+rule_a);
        for(auto& c : str){
            if(c=='<')tmp+="&lt;";
            else if(c=='&')tmp+="&amp;";
            else if(c=='\'')tmp+="&apos;";
            else tmp+=c;
        }
        return tmp;
    }
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
    int rule_a = 0; //Offset for 1-character escape sequiences
    int rule_b = 0; //Intermediate steps of ]]>
    int rule_c = 0; //Offset for rule_b
    for(auto& c : str){
        if(c=='<')rule_a+=sizeof("&lt;")-1-1;
        else if(c=='&')rule_a+=sizeof("&amp;")-1-1;
        else if(rule_b==0 && c==']')rule_b=1;
        else if(rule_b==1 && c==']')rule_b=2;
        else if(rule_b==2 && c=='>'){rule_b=0;rule_c+=sizeof("&gt;")-1-1;}   //]]> not allowed in text, it must be escaped as `]]&gt;`.
        else rule_b=0;
        //Maybe some config to enable the others as well even if not needed for a correct XML serialization?
    }
    if(rule_a==0 && rule_c==0)return str;
    else{
        std::string tmp;
        tmp.reserve(str.length()+rule_a+rule_c);
        int rule_b = 0;
        for(auto& c : str){
            if(rule_b==0 && c==']')rule_b=1;
            else if(rule_b==1 && c==']')rule_b=2;
            else if(rule_b==2 && c=='>'){rule_b=0;tmp+="]]&gt;";}   //]]> not allowed in text, it must be escaped as `]]&gt;`.
            else if(rule_b==1){rule_b=0;tmp+="]";}
            else if(rule_b==2){rule_b=0;tmp+="]]";}
            else if(c=='<')tmp+="&lt;";
            else if(c=='&')tmp+="&amp;";
            else tmp+=c;
        }
        return tmp;
    }
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


std::string_view unescape_xml(std::string_view sv) {
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
                    // Create a temporary std::string_view for the number.
                    std::string_view numStr(buffer + numStart, j - numStart);

                    // Convert to an integer.
                    int code = '?';
                    std::from_chars<int>(numStr.begin(),numStr.end(),code,hex ? 16 : 10);
                    buffer[write++] = static_cast<char>(code);
                    read = j + 1;
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


}}