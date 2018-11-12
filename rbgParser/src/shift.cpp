#include"shift.hpp"

namespace rbg_parser{

shift::shift(const token& edge_name):
edge_name(edge_name){
}

void shift::accept(abstract_dispatcher& dispatcher)const{
    dispatcher.dispatch(*this);
}

std::string shift::to_rbg(uint)const{
    return to_rbg();
}

std::unique_ptr<game_move> shift::flatten(void){
    return std::unique_ptr<game_move>(new shift(std::move(*this)));
}

std::string shift::to_rbg()const{
    std::string result = "";
    result += edge_name.to_string();
    result += " ";
    return result;
}

const token& shift::get_content(void)const{
    return edge_name;
}

std::unique_ptr<game_move> shift::copy(void)const{
    return std::unique_ptr<game_move>(new shift(edge_name));
}

}
