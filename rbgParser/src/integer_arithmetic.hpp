#ifndef INTEGER_ARITHMETIC
#define INTEGER_ARITHMETIC

#include"arithmetic_expression.hpp"

namespace rbg_parser{

class integer_arithmetic : public arithmetic_expression{
        uint content;
    public:
        integer_arithmetic(uint content);
        integer_arithmetic(const integer_arithmetic&)=delete;
        integer_arithmetic(integer_arithmetic&&)=default;
        integer_arithmetic& operator=(const integer_arithmetic&)=delete;
        integer_arithmetic& operator=(integer_arithmetic&&)=default;
        ~integer_arithmetic(void)override=default;
        void accept(abstract_dispatcher &dispatcher)const override;
        std::unique_ptr<arithmetic_expression> simplify(void)override;
        uint priority(void)const override{return 4;};
        std::string to_rbg(uint)const override;
        std::string to_rbg()const override;
        std::unique_ptr<arithmetic_expression> flatten(void)override;
        uint get_content(void)const;
        std::unique_ptr<arithmetic_expression> copy(void)const override;
};

}

#endif
