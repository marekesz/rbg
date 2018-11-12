#include<cstring>

#include"options.hpp"

namespace rbg_parser{

wrong_argument_error::wrong_argument_error(void):
std::exception(),
description("Wrong argument error"){
}

wrong_argument_error::wrong_argument_error(const std::string& source):
std::exception(),
description(source){
}

wrong_argument_error::~wrong_argument_error(void){
}

const char* wrong_argument_error::what(void)const noexcept{
    return description.c_str();
}

options::options(void):
show_warnings(true),
warnings_as_errors(false),
output_name("a.rbg"){
}

options::options(uint number_of_args, const char** args):
show_warnings(true),
warnings_as_errors(false),
output_name("a.rbg"){
    for(uint i=0;i<number_of_args;++i){
        if(args[i][0] != '-')
            throw wrong_argument_error("Every flag should begin with \"-\"");
        else{
            if(!std::strcmp(args[i], "-o")){
                ++i;
                if(i >= number_of_args)
                    throw wrong_argument_error("Flag \"-o\" should be succeeded by output file name");
                else
                    output_name = args[i];
            }
            else if(!std::strcmp(args[i], "-Whide"))
                show_warnings = false;
            else if(!std::strcmp(args[i], "-Werror"))
                warnings_as_errors = true;
            else
                throw wrong_argument_error("Unrecognized flag");
        }
    }
}

bool options::showing_warnings(void)const{
    return show_warnings;
}
bool options::escalating_warnings(void)const{
    return warnings_as_errors;
}

const std::string& options::output_file(void)const{
    return output_name;
}

}
