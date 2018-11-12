#ifndef OPTIONS
#define OPTIONS

#include<string>
#include<exception>

#include"types.hpp"

namespace rbg_parser{

class wrong_argument_error : public std::exception{
        std::string description;
    public:
        wrong_argument_error(void);
        wrong_argument_error(const std::string& description);
        virtual ~wrong_argument_error(void);

        virtual const char* what(void)const noexcept;
};

class options{
        bool show_warnings : 1;
        bool warnings_as_errors : 1;
        std::string output_name;
    public:
        options(void);
        options(uint number_of_args, const char** args);

        bool showing_warnings(void)const;
        bool escalating_warnings(void)const;
        const std::string& output_file(void)const;
};

}

#endif
