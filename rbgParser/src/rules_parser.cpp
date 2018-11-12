#include"rules_parser.hpp"
#include"tree_utils.hpp"
#include"tree_parser.hpp"
#include"slice_iterator.hpp"
#include"integer_leaf.hpp"
#include"identifier_leaf.hpp"
#include"arrow_leaf.hpp"
#include"typing_machine.hpp"

namespace rbg_parser{

parser_result<suffix> parse_power(slice_iterator& it, messages_container& msg){
    if(not it.has_value())
        return failure<suffix>();
    if(it.current(msg).get_type() == star){
        it.next(msg);
        return success(suffix{star_power,0});
    }
    else if(it.current(msg).get_type() == caret){
        it.next(msg);
        suffix result;
        if(it.current(msg).get_type() == number)
            result = suffix{number_power, it.current(msg).get_value()};
        else
            throw msg.build_message(it.create_call_stack("Expected number, encountered \'"+it.current(msg).to_string()+"\'"));
        it.next(msg);
        return success(std::move(result));
    }
    return failure<suffix>();
}

std::unique_ptr<expression> parse_rules(slice_iterator& it, const typing_machine& t, messages_container& msg){
    tree_parser p(operator_info{concatenate, 90, false}, t);

    p.add_suffix_parser(parse_power);
    p.add_leaf_parser(parse_integer_leaf);
    p.add_leaf_parser(parse_identifier_leaf);
    p.add_leaf_parser(parse_arrow_leaf);

    p.add_operator(plus, operator_info{add, 51, false});
    p.add_operator(minus, operator_info{subtract, 52, false});
    p.add_operator(star, operator_info{multiply, 53, false});
    p.add_operator(slash, operator_info{divide, 54, false});

    p.add_operator(comma, operator_info{separate, 30, false});
    p.add_operator(equal, operator_info{assign, 40, false});
    p.add_operator(less, operator_info{is_less, 41, false});
    p.add_operator(less_equal, operator_info{is_less_eq, 42, false});
    p.add_operator(greater, operator_info{is_greater, 43, false});
    p.add_operator(greater_equal, operator_info{is_greater_eq, 44, false});
    p.add_operator(double_equal, operator_info{is_equal, 45, false});
    p.add_operator(not_equal, operator_info{is_nequal, 46, false});

    p.add_bracket(left_round_bracket, bracket_info{right_round_bracket, ")", standard_bracket});
    p.add_bracket(left_question_bracket, bracket_info{right_curly_bracket, "}", condition_bracket});
    p.add_bracket(left_exclamation_bracket, bracket_info{right_curly_bracket, "}", negated_condition_bracket});
    p.add_bracket(left_dollar_bracket, bracket_info{right_curly_bracket, "}", comparison_bracket});
    p.add_bracket(left_square_bracket, bracket_info{right_square_bracket, "]", modifier_bracket});
    p.add_bracket(left_dollar_square_bracket, bracket_info{right_square_bracket, "]", assignment_bracket});
    p.add_bracket(left_curly_bracket, bracket_info{right_curly_bracket, "}", on_bracket});
    return p.parse_naked_expression(it, msg);
}

}
