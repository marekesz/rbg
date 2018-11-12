#include"ons.hpp"
#include"printer_helpers.hpp"

namespace rbg_parser{

ons::ons(std::vector<token>&& legal_ons):
legal_ons(legal_ons.begin(),legal_ons.end()){
}

void ons::accept(abstract_dispatcher& dispatcher)const{
    dispatcher.dispatch(*this);
}

std::string ons::to_rbg(uint)const{
    return to_rbg();
}

std::string ons::to_rbg()const{
    std::string result = "{";
    result += rbg_parser::to_rbg(legal_ons);
    result += "}";
    return result;
}

std::unique_ptr<game_move> ons::flatten(void){
    return std::unique_ptr<game_move>(new ons(std::move(*this)));
}

const std::set<token>& ons::get_legal_ons(void)const{
    return legal_ons;
}

std::unique_ptr<game_move> ons::copy(void)const{
    std::vector<token> result_legal_ons;
    for(const auto& el: legal_ons)
        result_legal_ons.push_back(el);
    return std::unique_ptr<game_move>(new ons(std::move(result_legal_ons)));
}

}
