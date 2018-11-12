#ifndef CPP_CONTAINER
#define CPP_CONTAINER

#include<string>
#include<vector>
#include<set>
#include<fstream>

class cpp_container{
        std::string name;
        std::set<std::string> header_includes;
        std::set<std::string> source_includes;
        std::vector<std::string> header_code;
        std::vector<std::string> source_code;
        void print_includes(std::set<std::string> cpp_container::*headers_type, std::ofstream& out)const;
        void print_code(std::vector<std::string> cpp_container::*code_type, std::ofstream& out)const;
        std::string name_as_macro(void)const;
        void print_header(void)const;
        void print_source(void)const;
    public:
        cpp_container(void)=default;
        cpp_container(const cpp_container&)=default;
        cpp_container(cpp_container&&)=default;
        cpp_container& operator=(const cpp_container&)=default;
        cpp_container& operator=(cpp_container&&)=default;
        ~cpp_container(void)=default;
        cpp_container(const std::string& name);
        void add_header_include(const std::string& file_name);
        void add_source_include(const std::string& file_name);
        void add_header_line(const std::string& line);
        void add_source_line(const std::string& line);
        void print_files(void)const;
};

#endif
