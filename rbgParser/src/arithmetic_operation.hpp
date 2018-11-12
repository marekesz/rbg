#ifndef ARITHMETIC_OPERATION
#define ARITHMETIC_OPERATION

#include"arithmetic_expression.hpp"
#include"token.hpp"
#include<vector>
#include<memory>

namespace rbg_parser{

enum operation{
    addition,
    subtraction,
    multiplication,
    division
};

class arithmetic_operation : public arithmetic_expression{
        std::vector<std::unique_ptr<arithmetic_expression>> content;
        operation kind_of_operation;
    public:
        arithmetic_operation(std::vector<std::unique_ptr<arithmetic_expression>>&& content, operation kind_of_operation);
        arithmetic_operation(const arithmetic_operation&)=delete;
        arithmetic_operation(arithmetic_operation&&)=default;
        arithmetic_operation& operator=(const arithmetic_operation&)=delete;
        arithmetic_operation& operator=(arithmetic_operation&&)=default;
        ~arithmetic_operation(void)override=default;
        void accept(abstract_dispatcher &dispatcher)const override;
        std::unique_ptr<arithmetic_expression> simplify(void)override;
        uint priority(void)const override;
        std::string to_rbg(uint)const override;
        std::string to_rbg()const override;
        std::unique_ptr<arithmetic_expression> flatten(void)override;
        void gather_sum_elements(std::vector<std::unique_ptr<arithmetic_expression>>& elements)override;
        void gather_multiply_elements(std::vector<std::unique_ptr<arithmetic_expression>>& elements)override;
        const std::vector<std::unique_ptr<arithmetic_expression>>& get_content(void)const;
        operation get_operation(void)const;
        std::unique_ptr<arithmetic_expression> copy(void)const override;
};

}

#endif
