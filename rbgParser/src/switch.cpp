#include"switch.hpp"

namespace rbg_parser{

player_switch::player_switch(const token& to_player):
to_player(to_player){
}

void player_switch::accept(abstract_dispatcher& dispatcher)const{
    dispatcher.dispatch(*this);
}

std::string player_switch::to_rbg(uint)const{
    return to_rbg();
}

std::string player_switch::to_rbg()const{
    return "->" + to_player.to_string();
}

std::unique_ptr<game_move> player_switch::flatten(void){
    return std::unique_ptr<game_move>(new player_switch(std::move(*this)));
}

straightness_result player_switch::compute_k_straightness(void)const{
    return standard_switch();
}

const token& player_switch::get_player(void)const{
    return to_player;
}

std::unique_ptr<game_move> player_switch::copy(void)const{
    return std::unique_ptr<game_move>(new player_switch(to_player));
}

keeper_switch::keeper_switch(bool deterministic):
deterministic(deterministic){
}

void keeper_switch::accept(abstract_dispatcher& dispatcher)const{
    dispatcher.dispatch(*this);
}

std::string keeper_switch::to_rbg(uint)const{
    return to_rbg();
}

std::string keeper_switch::to_rbg()const{
    return is_deterministic() ? "->>" : "->*";
}

std::unique_ptr<game_move> keeper_switch::flatten(void){
    return std::unique_ptr<game_move>(new keeper_switch(std::move(*this)));
}

straightness_result keeper_switch::compute_k_straightness(void)const{
    return standard_switch();
}

bool keeper_switch::is_deterministic(void)const{
    return deterministic;
}

std::unique_ptr<game_move> keeper_switch::copy(void)const{
    return std::unique_ptr<game_move>(new keeper_switch(deterministic));
}

}
