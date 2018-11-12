#include"variable_arithmetic.hpp"
#include"abstract_dispatcher.hpp"

namespace rbg_parser{

variable_arithmetic::variable_arithmetic(const token& content):
content(content){
}

void variable_arithmetic::accept(abstract_dispatcher &dispatcher)const{
    dispatcher.dispatch(*this);
}

std::unique_ptr<arithmetic_expression> variable_arithmetic::simplify(void){
    return std::unique_ptr<arithmetic_expression>(new variable_arithmetic(std::move(*this)));
}

std::unique_ptr<arithmetic_expression> variable_arithmetic::flatten(void){
    return std::unique_ptr<arithmetic_expression>(new variable_arithmetic(std::move(*this)));
}

std::string variable_arithmetic::to_rbg(uint)const{
    return to_rbg();
}

std::string variable_arithmetic::to_rbg()const{
    return content.to_string();
}

const token& variable_arithmetic::get_content(void)const{
    return content;
}

std::unique_ptr<arithmetic_expression> variable_arithmetic::copy(void)const{
    return std::unique_ptr<arithmetic_expression>(new variable_arithmetic(content));
}

}
