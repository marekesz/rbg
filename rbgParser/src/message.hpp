#ifndef MESSAGE
#define MESSAGE

#include<string>
#include<vector>
#include<iostream>

#include"types.hpp"

namespace rbg_parser{

class message{
        std::vector<std::pair<std::pair<uint,uint>,std::string>> call_stack;
    public:
        message(uint line, uint character, const std::string& details)noexcept;
        message(std::vector<std::pair<std::pair<uint,uint>,std::string>>&& cs)noexcept;

        std::string as_warning(void)const;
        std::string as_error(void)const;
};

class messages_container{
        std::vector<uint> new_lines_positions;
        std::vector<message> warnings_list;
        uint find_line_number(uint character)const;
    public:
        messages_container(void)noexcept;

        void add_message(uint character, const std::string& details);
        message build_message(uint character, const std::string& details)const;
        void add_message(const std::string& details);
        message build_message(const std::string& details)const;
        void add_message(const std::vector<std::pair<uint,std::string>>& call_stack);
        message build_message(const std::vector<std::pair<uint,std::string>>& call_stack)const;
        void report_new_line(uint character);
        void write_as_warnings(std::ostream& out)const;
        void write_as_errors(std::ostream& out)const;
        bool is_empty(void)const;
};

}

#endif
