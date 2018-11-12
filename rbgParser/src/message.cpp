#include<cassert>

#include"message.hpp"

namespace rbg_parser{

message::message(uint l, uint c, const std::string& d)noexcept:
call_stack(){
    call_stack.push_back(std::make_pair(std::make_pair(l,c),d));
}

message::message(std::vector<std::pair<std::pair<uint,uint>,std::string>>&& cs)noexcept:
call_stack(std::move(cs)){
}

std::string message::as_warning(void)const{
    std::string result;
    for(uint i=0;i<call_stack.size();++i){
        if(call_stack[i].first.second>0)
            result += std::string(i>0 ? "\n\t" : "")+"Warning (line "+std::to_string(call_stack[i].first.first)+", character "+std::to_string(call_stack[i].first.second)+"): "+call_stack[i].second;
        else
            result += std::string(i>0 ? "\n\t" : "")+"Warning: "+call_stack[i].second;
    }
    return result;
}

std::string message::as_error(void)const{
    std::string result;
    for(uint i=0;i<call_stack.size();++i){
        if(call_stack[i].first.second>0)
            result += std::string(i>0 ? "\n\t" : "")+"Error (line "+std::to_string(call_stack[i].first.first)+", character "+std::to_string(call_stack[i].first.second)+"): "+call_stack[i].second;
        else
            result += std::string(i>0 ? "\n\t" : "")+"Error: "+call_stack[i].second;
    }
    return result;
}

uint messages_container::find_line_number(uint character)const{
    assert(character>0);
    uint begin = 0;
    uint end = new_lines_positions.size();
    uint middle;
    while(middle = (begin + end)/2,(new_lines_positions[middle]>character || (middle<new_lines_positions.size()-1 && new_lines_positions[middle+1]<=character))){
        if(new_lines_positions[middle]>character)
            end = middle;
        else
            begin = middle+1;
    }
    return middle+1;
}

messages_container::messages_container(void)noexcept:
new_lines_positions(),
warnings_list(){
    new_lines_positions.push_back(1);
}

void messages_container::add_message(uint character, const std::string& details){
    warnings_list.push_back(build_message(character,details));
}

message messages_container::build_message(uint character, const std::string& details)const{
    uint line = find_line_number(character+1);
    return message(line,character+2-new_lines_positions[line-1],details);
}

void messages_container::add_message(const std::string& details){
    warnings_list.push_back(build_message(details));
}

message messages_container::build_message(const std::string& details)const{
    return message(0,0,details);
}

void messages_container::add_message(const std::vector<std::pair<uint,std::string>>& call_stack){
    warnings_list.push_back(build_message(call_stack));
}

message messages_container::build_message(const std::vector<std::pair<uint,std::string>>& call_stack)const{
    std::vector<std::pair<std::pair<uint,uint>,std::string>> result;
    for(const auto& el: call_stack){
        uint line = find_line_number(el.first+1);
        result.push_back(std::make_pair(std::make_pair(line,el.first+2-new_lines_positions[line-1]),el.second));
    }
    return message(std::move(result));
}

void messages_container::report_new_line(uint character){
    assert(character>=new_lines_positions.back());
    new_lines_positions.push_back(character+1);
}

void messages_container::write_as_warnings(std::ostream& out)const{
    for(const auto& el: warnings_list)
        out<<el.as_warning()<<std::endl;
}

void messages_container::write_as_errors(std::ostream& out)const{
    for(const auto& el: warnings_list)
        out<<el.as_error()<<std::endl;
}

bool messages_container::is_empty(void)const{
    return warnings_list.empty();
}

}
