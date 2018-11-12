#include"integer_arithmetic.hpp"
#include"abstract_dispatcher.hpp"

namespace rbg_parser{

integer_arithmetic::integer_arithmetic(uint content):
content(content){
}

void integer_arithmetic::accept(abstract_dispatcher &dispatcher)const{
    dispatcher.dispatch(*this);
}

std::unique_ptr<arithmetic_expression> integer_arithmetic::simplify(void){
    return std::unique_ptr<arithmetic_expression>(new integer_arithmetic(content));
}

std::unique_ptr<arithmetic_expression> integer_arithmetic::flatten(void){
    return std::unique_ptr<arithmetic_expression>(new integer_arithmetic(content));
}

std::string integer_arithmetic::to_rbg(uint)const{
    return to_rbg();
}

std::string integer_arithmetic::to_rbg()const{
    return std::to_string(content);
}

uint integer_arithmetic::get_content(void)const{
    return content;
}

std::unique_ptr<arithmetic_expression> integer_arithmetic::copy(void)const{
    return std::unique_ptr<arithmetic_expression>(new integer_arithmetic(content));
}

}
