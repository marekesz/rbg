#include"arithmetic_comparison.hpp"
#include"printer_helpers.hpp"

namespace rbg_parser{

arithmetic_comparison::arithmetic_comparison(
    std::unique_ptr<arithmetic_expression> left_side,
    kind kind_of_comparison,
    std::unique_ptr<arithmetic_expression> right_side):
left_side(std::move(left_side)),
kind_of_comparison(kind_of_comparison),
right_side(std::move(right_side)){
}

void arithmetic_comparison::accept(abstract_dispatcher& dispatcher)const{
    dispatcher.dispatch(*this);
}

std::string arithmetic_comparison::to_rbg(uint)const{
    return to_rbg();
}

std::string arithmetic_comparison::to_rbg()const{
    std::string result = "{$ ";
    result += left_side->to_rbg();
    switch(kind_of_comparison){
        case eq:
            result += "==";
            break;
        case neq:
            result += "!=";
            break;
        case le:
            result += "<";
            break;
        case leq:
            result += "<=";
            break;
        case ge:
            result += ">";
            break;
        case geq:
            result += "<=";
            break;
        default:
            break;
    }
    result += right_side->to_rbg();
    result += "}";
    return result;
}

std::unique_ptr<game_move> arithmetic_comparison::simplify(void){
    auto new_left_side = left_side->simplify();
    auto new_right_side = right_side->simplify();
    return std::unique_ptr<game_move>(new arithmetic_comparison(std::move(new_left_side), kind_of_comparison, std::move(new_right_side)));
}

std::unique_ptr<game_move> arithmetic_comparison::flatten(void){
    auto new_left_side = left_side->flatten();
    auto new_right_side = right_side->flatten();
    return std::unique_ptr<game_move>(new arithmetic_comparison(std::move(new_left_side), kind_of_comparison, std::move(new_right_side)));
}

const arithmetic_expression* arithmetic_comparison::get_left_side(void)const{
    return left_side.get();
}

kind arithmetic_comparison::get_kind_of_comparison(void)const{
    return kind_of_comparison;
}

const arithmetic_expression* arithmetic_comparison::get_right_side(void)const{
    return right_side.get();
}

std::unique_ptr<game_move> arithmetic_comparison::copy(void)const{
    return std::unique_ptr<game_move>(new arithmetic_comparison(left_side->copy(),kind_of_comparison,right_side->copy()));
}

}
