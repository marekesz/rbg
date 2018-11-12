#include"printer_helpers.hpp"

namespace rbg_parser{

std::string to_rbg(const std::vector<token>& v){
    std::string result = "";
    for(const auto& el: v)
        result+=el.to_string()+",";
    if(not result.empty())
        result.pop_back();
    return result;
}

std::string to_rbg(const std::set<token>& s){
    std::string result = "";
    for(const auto& el: s)
        result+=el.to_string()+",";
    if(not result.empty())
        result.pop_back();
    return result;
}

std::string to_rbg(const std::map<token, uint>& s){
    std::string result = "";
    for(const auto& el: s)
        result+=el.first.to_string()+"("+std::to_string(el.second)+"),";
    if(not result.empty())
        result.pop_back();
    return result;
}

std::string print_tabs(uint indent){
    std::string result = "";
    for(uint i=0;i<indent;++i)
        result += "    ";
    return result;
}

std::string print_tabs_with_half_full_end(uint indent){
    std::string result = "";
    for(uint i=0;i<indent-1;++i)
        result += "    ";
    if(indent>0)
        result += "  ";
    return result;
}

std::string open_bracket_if_necessary(uint current_priority,uint lower_priority){
    return (current_priority > lower_priority ? "(" : "");
}

std::string close_bracket_if_necessary(uint current_priority,uint lower_priority){
    return (current_priority > lower_priority ? ")" : "");
}

std::string print_variable(const token& v){
    return v.to_string();
}

}
