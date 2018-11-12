#include"move_check.hpp"

namespace rbg_parser{

move_check::move_check(std::unique_ptr<game_move>&& content, bool negated):
content(std::move(content)),
negated(negated){
}

void move_check::accept(abstract_dispatcher& dispatcher)const{
    dispatcher.dispatch(*this);
}

const game_move* move_check::get_content(void)const{
    return content.get();
}

bool move_check::is_negated(void)const{
    return negated;
}

std::unique_ptr<game_move> move_check::simplify(void){
    return std::unique_ptr<game_move>(new move_check(content->simplify(), negated));
}

straightness_result move_check::compute_k_straightness(void)const{
    auto result = content->compute_k_straightness();
    result.wrap_in_check();
    return result;
}

std::string move_check::to_rbg(uint indent)const{
    std::string result = "";
    result += negated ? "{!" : "{?";
    result += content->to_rbg(indent);
    result += "}";
    return result;
}

std::string move_check::to_rbg()const{
    std::string result = "";
    result += negated ? "{!" : "{?";
    result += content->to_rbg();
    result += "}";
    return result;
}

std::unique_ptr<game_move> move_check::flatten(void){
    return std::unique_ptr<game_move>(new move_check(content->flatten(), negated));
}

std::unique_ptr<game_move> move_check::copy(void)const{
    return std::unique_ptr<game_move>(new move_check(content->copy(),negated));
}

}
