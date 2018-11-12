#include"cpp_container.hpp"

#include<stdexcept>

cpp_container::cpp_container(const std::string& name):
name(name),
header_includes(),
source_includes(),
header_code(),
source_code(){
}

void cpp_container::add_header_include(const std::string& file_name){
    header_includes.insert(file_name);
}

void cpp_container::add_source_include(const std::string& file_name){
    source_includes.insert(file_name);
}

void cpp_container::add_header_line(const std::string& line){
    header_code.push_back(line);
}

void cpp_container::add_source_line(const std::string& line){
    source_code.push_back(line);
}

void cpp_container::print_includes(std::set<std::string> cpp_container::*headers_type, std::ofstream& out)const{
    for(const auto& el: this->*headers_type)
        out<<"#include <"+el+">\n";
    out<<"\n";
}

void cpp_container::print_code(std::vector<std::string> cpp_container::*code_type, std::ofstream& out)const{
    uint indent = 0;
    std::vector<uint> part_of_case;
    for(const auto& el: this->*code_type){
        if(el.empty())
            out<<"\n";
        else{
            if(el[0] == '}' or el.back() == ':')
                if(not part_of_case.empty() and part_of_case.back() == indent)
                    part_of_case.pop_back();
            if(el[0] == '}')
                --indent;
            for(uint i=0;i<indent+part_of_case.size();++i)
                out<<"    ";
            out<<el<<"\n";
            if(el.back() == '{')
                ++indent;
            if(el.back() == ':' and (part_of_case.empty() or part_of_case.back() != indent))
                part_of_case.push_back(indent);
        }
    }
}

std::string cpp_container::name_as_macro(void)const{
    std::string result;
    for(const auto el: name)
        result.push_back(toupper(el));
    return result;
}

void cpp_container::print_header(void)const{
    std::ofstream out(name+".hpp");
    if(not out.good())
        throw std::invalid_argument("Couldn't open file "+name+".hpp");
    out<<"#ifndef "<<name_as_macro()<<"\n";
    out<<"#define "<<name_as_macro()<<"\n\n";
    print_includes(&cpp_container::header_includes, out);
    print_code(&cpp_container::header_code, out);
    out<<"\n#endif\n";
}

void cpp_container::print_source(void)const{
    std::ofstream out(name+".cpp");
    if(not out.good())
        throw std::invalid_argument("Couldn't open file "+name+".cpp");
    out<<"#include \""+name+".hpp\"\n\n";
    print_includes(&cpp_container::source_includes, out);
    print_code(&cpp_container::source_code, out);
}

void cpp_container::print_files(void)const{
    print_header();
    print_source();
}
