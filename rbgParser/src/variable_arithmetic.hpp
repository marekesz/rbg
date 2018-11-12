#ifndef VARIABLE_ARITHMETIC
#define VARIABLE_ARITHMETIC

#include"arithmetic_expression.hpp"
#include"token.hpp"

namespace rbg_parser{

class variable_arithmetic : public arithmetic_expression{
        token content;
    public:
        variable_arithmetic(const token& content);
        variable_arithmetic(const variable_arithmetic&)=delete;
        variable_arithmetic(variable_arithmetic&&)=default;
        variable_arithmetic& operator=(const variable_arithmetic&)=delete;
        variable_arithmetic& operator=(variable_arithmetic&&)=default;
        ~variable_arithmetic(void)override=default;
        void accept(abstract_dispatcher &dispatcher)const override;
        std::unique_ptr<arithmetic_expression> simplify(void)override;
        uint priority(void)const override{return 4;};
        std::string to_rbg(uint)const override;
        std::string to_rbg()const override;
        std::unique_ptr<arithmetic_expression> flatten(void)override;
        const token& get_content(void)const;
        std::unique_ptr<arithmetic_expression> copy(void)const override;
};

}

#endif
