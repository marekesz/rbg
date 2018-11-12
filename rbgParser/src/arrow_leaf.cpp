#include"arrow_leaf.hpp"
#include"slice_iterator.hpp"
#include"message.hpp"
#include"typing_machine.hpp"
#include"switch.hpp"

namespace rbg_parser{

arrow_leaf::arrow_leaf(slice_iterator&& beginning_position, const token& next, bool has_next):
expression(std::move(beginning_position)),
next(next),
has_next(has_next){
}

expression_type arrow_leaf::get_type(void)const{
    return gmove;
}

void arrow_leaf::type(const typing_machine& t, messages_container& msg){
    if(has_next and not t.can_be_switch(next))
        throw msg.build_message(beginning_position.create_call_stack("Player "+next.to_string()+" was not declared"));
}

std::unique_ptr<game_move> arrow_leaf::get_game_move(void)const{
    if(has_next){
        if(next.get_type() == star)
            return std::unique_ptr<game_move>(new keeper_switch(false));
        else
            return std::unique_ptr<game_move>(new player_switch(next));
    }
    else
        return std::unique_ptr<game_move>(new keeper_switch(true));
}

parser_result<std::unique_ptr<expression>> parse_arrow_leaf(slice_iterator& it, messages_container& msg){
    parsing_context_string_guard g(&it, "Unexpected end of input while parsing arrow expression");
    auto beginning = it;
    if(not it.has_value())
        return failure<std::unique_ptr<expression>>();
    if(it.current(msg).get_type() == keeper_arrow){
        it.next(msg);
        return success(std::unique_ptr<expression>(new arrow_leaf(std::move(beginning), token(), false)));
    }
    else if(it.current(msg).get_type() == arrow){
        it.next(msg);
        if(it.current(msg).get_type() == identifier){
            std::unique_ptr<expression> result(new arrow_leaf(std::move(beginning), it.current(msg)));
            it.next(msg);
            return success(std::move(result));
        }
        throw msg.build_message(it.create_call_stack("Expected identifier, encountered \'"+it.current(msg).to_string()+"\'"));
    }
    else
        return failure<std::unique_ptr<expression>>();
}

}
