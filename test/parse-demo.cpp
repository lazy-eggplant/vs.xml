#include <string_view>
#include <vs-xml/parser.hpp>
#include <print>

auto parse(std::string_view src){
    enum struct error_t{
        OK,
        BAD_SPECIAL_SEQ,
    };

    enum struct state_t{
        S0,
        S1,
        Sb2 /*PROC_BEG*/ ,S3,
        Se2 /*PROC_END*/,
        Sb4_1, Sb4, /*COMMENT_BEG*/
        Se4_1, Se4_2, /*COMMENT_END*/
        Sb5_1, Sb5_2, Sb5_3, Sb5_4, Sb5_5, Sb5_6, Sb5 /*CDATA_BEG*/,
        Se5_1, Se5_2, /*CDATA_END*/
        ERROR
    }state=state_t::S0;

    for(auto c : src){
        if(state==state_t::S0 and (c==' ' or c=='\t' or c=='\n' or c=='\r')){}
        else if(state==state_t::S0){
            if(c=='<'){state=state_t::S1;}
            else if(c=='&'){

            }
            else{/*ADD_TEXT*/}
        }
        else if(state==state_t::S1){
            if(c=='?') /*PROC*/ {state=state_t::Sb2;}
            else if(c=='!') /*COMMENT,CDATA/*/ {state=state_t::S3;}
        }
        else if(state==state_t::Sb2){
            if(c=='?'){state=state_t::Se2;}
            else{/*ADD_PROC*/}
        }
        else if(state==state_t::Se2){
            if(c=='>'){state=state_t::S0;}
            else{/*ADD_PROC+?*/}
        }
        else if(state==state_t::S3){
            if(c=='-'){state=state_t::Sb4_1;}
            if(c=='['){state=state_t::Sb5_1;}
            else{return error_t::BAD_SPECIAL_SEQ;}
        }
        else if(state==state_t::Sb4_1){
            if(c=='-'){state=state_t::Sb4;}
            else{return error_t::BAD_SPECIAL_SEQ;}
        }
        else if(state==state_t::Sb4){
            if(c=='-'){state=state_t::Se4_1;}
            else{/*ADD_COMMENT*/}
        }
        else if(state==state_t::Se4_1){
            if(c=='-'){state=state_t::Se4_2;}
            else{/*ADD_COMMENT+-*/}
        }
        else if(state==state_t::Se4_2){
            if(c=='>'){state=state_t::S0;}
            else{/*ADD_COMMENT+--*/}
        }
        else if(state==state_t::Sb5_1){
            if(c=='C'){state=state_t::Sb5_2;}
            else{return error_t::BAD_SPECIAL_SEQ;}
        }
        else if(state==state_t::Sb5_2){
            if(c=='D'){state=state_t::Sb5_3;}
            else{return error_t::BAD_SPECIAL_SEQ;}
        }
        else if(state==state_t::Sb5_3){
            if(c=='A'){state=state_t::Sb5_4;}
            else{return error_t::BAD_SPECIAL_SEQ;}
        }
        else if(state==state_t::Sb5_4){
            if(c=='T'){state=state_t::Sb5_5;}
            else{return error_t::BAD_SPECIAL_SEQ;}
        }
        else if(state==state_t::Sb5_5){
            if(c=='A'){state=state_t::Sb5_6;}
            else{return error_t::BAD_SPECIAL_SEQ;}
        }
        else if(state==state_t::Sb5_6){
            if(c=='['){state=state_t::Sb5;}
            else{return error_t::BAD_SPECIAL_SEQ;}
        }
        else if(state==state_t::Sb5){
            if(c==']'){state=state_t::Se5_1;}
            else{/*ADD_CDATA*/}
        }
        else if(state==state_t::Se5_1){
            if(c==']'){state=state_t::Se5_2;}
            else{/*ADD_CDATA+]*/}
        }
        else if(state==state_t::Se5_2){
            if(c=='>'){state=state_t::S0;}
            else{/*ADD_CDATA+]]*/}
        }
    }

    return error_t::OK;
}

int main(){
    constexpr char file[] = {
        #embed "assets/demo-0.xml"
        ,'\0'
    };
    
    std::string_view file_sv{file, file+sizeof(file)};

    xml::Parser parser;

    //tree.print(std::cout,{});
    std::print("{}",file);
    std::print("\n");
    return 0;
}