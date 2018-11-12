#ifndef ARITHMETIC_EXPRESSION
#define ARITHMETIC_EXPRESSION

#include<memory>
#include<vector>

namespace rbg_parser{

class abstract_dispatcher;
// interface
class arithmetic_expression{
    public:
        virtual ~arithmetic_expression(void)=default;
        virtual void accept(abstract_dispatcher &dispatcher)const=0;
        virtual std::unique_ptr<arithmetic_expression> simplify(void)=0;
        virtual uint priority(void)const=0; // being of higher priority containing lower ones requires surrounding them with brackets
        virtual std::string to_rbg(uint indent)const=0;
        virtual std::string to_rbg()const=0;
        virtual std::unique_ptr<arithmetic_expression> flatten(void)=0;
        virtual void gather_sum_elements(std::vector<std::unique_ptr<arithmetic_expression>>& elements){elements.push_back(flatten());};
        virtual void gather_multiply_elements(std::vector<std::unique_ptr<arithmetic_expression>>& elements){elements.push_back(flatten());};
        virtual std::unique_ptr<arithmetic_expression> copy(void)const=0;
};

}

#endif
