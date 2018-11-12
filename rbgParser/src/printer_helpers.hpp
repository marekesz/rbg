#ifndef PRINTER_HELPERS
#define PRINTER_HELPERS

#include<vector>
#include<set>
#include<map>
#include<string>

#include"token.hpp"
#include"types.hpp"

namespace rbg_parser{

std::string to_rbg(const std::vector<token>&);
std::string to_rbg(const std::set<token>&);
std::string to_rbg(const std::map<token, uint>&);
std::string print_tabs(uint indent);
std::string print_tabs_with_half_full_end(uint indent);
std::string open_bracket_if_necessary(uint current_priority,uint lower_priority);
std::string close_bracket_if_necessary(uint current_priority,uint lower_priority);
std::string print_variable(const token& v);

}

#endif
