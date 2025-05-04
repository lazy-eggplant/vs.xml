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

namespace xml{
namespace serialize{
//https://stackoverflow.com/questions/1091945/what-characters-do-i-need-to-escape-in-xml-documents
//An attempt will be made to keep the input string_view, unless escaping is needed.

typedef std::optional<std::variant<std::string,std::string_view>> ret_t;

constexpr std::optional<std::string_view> validate_xml_label(std::string_view str){
    int pos = 0;
    for(auto& c : str){
        if(pos==0 && (c=='_' or (c>'a' && c<'z') or (c>'A' && c<'Z'))){/*OK*/}
        //In theory some intervals of utf8 should be negated. But this filter is good enough for now.
        else if(pos!=0 && (c=='_' or c=='.' or c=='-' or (c>='0' && c<='9') or (c>='a' && c<='z') or (c>='A' && c<='Z') or (c>127))){/*OK*/}
        else{
            //std::print("{} @ pos {}----- {} {} {} \n",(int)c, pos, str, (void*)str.data(), str.length());
            return {};
        }
        pos++;
    }
    return str;
}

constexpr ret_t to_xml_attr_1(std::string_view str){
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

constexpr ret_t to_xml_attr_2(std::string_view str){
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

constexpr ret_t to_xml_text(std::string_view str){
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

constexpr ret_t to_xml_cdata(std::string_view str){
    int rule_a = 0;
    for(auto& c : str){
        if(rule_a==0 && c==']')rule_a=1;
        else if(rule_a==1 && c==']')rule_a=2;
        else if(rule_a==2 && c=='>')return {};   //Disallowed sequence ]]>
        else rule_a=0;
    }
    return str;
}

constexpr ret_t to_xml_comment(std::string_view str){
    int rule_a = 0;
    for(auto& c : str){
        if(rule_a==0 && c=='-')rule_a=1;
        else if(rule_a==1 && c=='-')return {};   //Disallowed sequence --
        else rule_a=0;
    }
    return str;
}

constexpr ret_t to_xml_proc(std::string_view str){
    int rule_a = 0;
    for(auto& c : str){
        if(rule_a==0 && c=='?')rule_a=1;
        else if(rule_a==1 && c=='>')return {};   //Disallowed sequence ?>
        else rule_a=0;
    }
    return str;
}

}
}