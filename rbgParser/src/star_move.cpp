#include<cassert>

#include"star_move.hpp"
#include"printer_helpers.hpp"

namespace rbg_parser{

star_move::star_move(std::unique_ptr<game_move>&& contained_move):
contained_move(std::move(contained_move)){
}

bool star_move::modifies(void)const{
    if(contained_move)
        return contained_move->modifies();
    else
        return false;
}

std::unique_ptr<game_move> star_move::simplify(void){
    std::unique_ptr<game_move> content = contained_move->simplify();
    return std::unique_ptr<game_move>(new star_move(std::move(content)));
}

void star_move::accept(abstract_dispatcher& dispatcher)const{
    dispatcher.dispatch(*this);
}

std::string star_move::to_rbg(uint indent)const{
    std::string result = "";
    result += open_bracket_if_necessary(priority(),contained_move->priority());
    result += contained_move->to_rbg(indent);
    result += close_bracket_if_necessary(priority(),contained_move->priority());
    result += "*";
    return result;
}

std::string star_move::to_rbg()const{
    std::string result = "";
    result += open_bracket_if_necessary(priority(),contained_move->priority());
    result += contained_move->to_rbg();
    result += close_bracket_if_necessary(priority(),contained_move->priority());
    result += "*";
    return result;
}

std::unique_ptr<game_move> star_move::flatten(void){
    return std::unique_ptr<game_move>(new star_move(contained_move->flatten()));
}

straightness_result star_move::compute_k_straightness(void)const{
    auto content_result = contained_move->compute_k_straightness();
    content_result.repeat_result();
    return content_result;
}

const game_move* star_move::get_content(void)const{
    return contained_move.get();
}

std::unique_ptr<game_move> star_move::copy(void)const{
    return std::unique_ptr<game_move>(new star_move(contained_move->copy()));
}

}
