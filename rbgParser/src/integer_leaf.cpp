#include"integer_leaf.hpp"
#include"slice_iterator.hpp"
#include"message.hpp"
#include"typing_machine.hpp"
#include"integer_arithmetic.hpp"

namespace rbg_parser{

integer_leaf::integer_leaf(slice_iterator&& beginning_position, int value):
expression(std::move(beginning_position)),
value(value){
}

expression_type integer_leaf::get_type(void)const{
    return value < 0 ? integer : unsigned_integer;
}

void integer_leaf::type(const typing_machine&, messages_container&){
}

uint integer_leaf::get_uint(void)const{
    assert(get_type() == unsigned_integer);
    return uint(value);
}

int integer_leaf::get_int(void)const{
    return value;
}

std::unique_ptr<arithmetic_expression> integer_leaf::get_arithmetic_expression(void)const{
    assert(get_type() == unsigned_integer);
    return std::unique_ptr<arithmetic_expression>(new integer_arithmetic(value));
}

parser_result<std::unique_ptr<expression>> parse_integer_leaf(slice_iterator& it, messages_container& msg){
    parsing_context_string_guard g(&it, "Unexpected end of input while parsing integer");
    auto beginning = it;
    if(not it.has_value())
        return failure<std::unique_ptr<expression>>();
    bool is_positive = true;
    if(it.current(msg).get_type() == plus)
        it.next(msg);
    else if(it.current(msg).get_type() == minus){
        is_positive = false;
        it.next(msg);
    }
    else if(it.current(msg).get_type() != number)
        return failure<std::unique_ptr<expression>>();
    if(it.current(msg).get_type() != number)
        throw msg.build_message(it.create_call_stack("Expected digit, encountered \'"+it.current(msg).to_string()+"\'"));
    std::unique_ptr<expression> result(new integer_leaf(std::move(beginning), is_positive ? int(it.current(msg).get_value()) : -int(it.current(msg).get_value())));
    it.next(msg);
    return success(std::move(result));
}

}
