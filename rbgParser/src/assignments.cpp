#include"assignments.hpp"
#include"printer_helpers.hpp"
#include"concatenation.hpp"

namespace rbg_parser{

assignment::assignment(const token& left_side, std::unique_ptr<arithmetic_expression>&& right_side):
left_side(left_side),
right_side(std::move(right_side)){
}

void assignment::accept(abstract_dispatcher& dispatcher)const{
    dispatcher.dispatch(*this);
}

std::string assignment::to_rbg(uint)const{
    return to_rbg();
}

std::string assignment::to_rbg()const{
    std::string result = "";
    result += "[$ ";
    result += print_variable(left_side);
    result += "=";
    result += right_side->to_rbg();
    result += "]";
    return result;
}

std::unique_ptr<game_move> assignment::simplify(void){
    auto new_right_side = right_side->simplify();
    return std::unique_ptr<game_move>(new assignment(std::move(left_side), std::move(new_right_side)));
}

std::unique_ptr<game_move> assignment::flatten(void){
    auto new_right_side = right_side->flatten();
    return std::unique_ptr<game_move>(new assignment(std::move(left_side), std::move(new_right_side)));
}

straightness_result assignment::compute_k_straightness(void)const{
    return modifier_non_switch();
}

bool assignment::check_if_redundant(std::set<token>& assignments_so_far, bool&)const{
    if(assignments_so_far.find(left_side) != assignments_so_far.end())
        return true;
    assignments_so_far.insert(left_side);
    return false;
}

const token& assignment::get_left_side(void)const{
    return left_side;
}

const arithmetic_expression* assignment::get_right_side(void)const{
    return right_side.get();
}

std::unique_ptr<game_move> make_assignments_concatenation(
    std::vector<std::pair<token, std::unique_ptr<arithmetic_expression>>>&& legal_sides){
    concatenation* result = new concatenation;
    for(auto& el: legal_sides)
        result->add_move(std::unique_ptr<game_move>(new assignment(std::move(el.first), std::move(el.second))));
    return std::unique_ptr<game_move>(result);
}

std::unique_ptr<game_move> assignment::copy(void)const{
    return std::unique_ptr<game_move>(new assignment(left_side, right_side->copy()));
}

}
