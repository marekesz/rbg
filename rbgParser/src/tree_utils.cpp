#include"tree_utils.hpp"

namespace rbg_parser{

expression::expression(slice_iterator&& beginning_position):
    beginning_position(std::move(beginning_position)){
}

const slice_iterator& expression::get_beginning(void)const{
    return beginning_position;
}

std::string bracket_type_description(bracket_type t){
    switch(t){
        case no_brackets:
            return "no brackets";
        case standard_bracket:
            return "standard brackets";
        case condition_bracket:
            return "condition brackets";
        case negated_condition_bracket:
            return "negated condtition brackets";
        case modifier_bracket:
            return "modifier bracket";
        case assignment_bracket:
            return "assignment bracket";
        case comparison_bracket:
            return "comparison bracket";
        case on_bracket:
            return "curly on bracket";
        default:
            return "";
    }
}

std::string expression_type_description(expression_type t){
    switch(t){
        case not_typed_yet:
            return "not typed expression";
        case error_type:
            return "error type";
        case integer:
            return "integer";
        case unsigned_integer:
            return "unsigned integer";
        case variable:
            return "variable";
        case piece_name:
            return "piece name";
        case player_name:
            return "player name";
        case edge_name:
            return "edge name";
        case single_assignment:
            return "assignment";
        case assignments_sequence:
            return "assignments sequence";
        case pieces_sequence:
            return "pieces sequence";
        case integer_comparison:
            return "integer comparison";
        case arithmetics:
            return "arithmetic expression";
        case mcheck:
            return "move check";
        case arithmetic_check:
            return "arithmetic check";
        case shift_move:
            return "shift move";
        case offs_move:
            return "offs move";
        case on_move:
            return "on move";
        case assignments_move:
            return "assignments move";
        case gmove:
            return "move expression";
        default:
            return "";
    }
}

bool is_subtype(expression_type t, expression_type sub_t){
    switch(t){
        case integer:
            return sub_t == integer
                || sub_t == unsigned_integer;
        case arithmetics:
            return sub_t == unsigned_integer
                || sub_t == variable
                || sub_t == piece_name
                || sub_t == player_name
                || sub_t == arithmetics;
        case variable:
            return sub_t == player_name
                || sub_t == variable;
        case pieces_sequence:
            return sub_t == piece_name
                || sub_t == pieces_sequence;
        case assignments_sequence:
            return sub_t == single_assignment
                || sub_t == assignments_sequence;
        case gmove:
            return sub_t == mcheck
                || sub_t == arithmetic_check
                || sub_t == shift_move
                || sub_t == on_move
                || sub_t == offs_move
                || sub_t == assignments_move
                || sub_t == gmove;
        default:
            return t == sub_t;
    }
}

std::string operator_type_description(operator_type t){
    switch(t){
        case no_operator:
            return "no operator";
        case concatenate:
            return "concatenation";
        case add:
            return "addition";
        case subtract:
            return "subtraction";
        case multiply:
            return "multiplication";
        case divide:
            return "division";
        case separate:
            return "comma";
        case assign:
            return "assignment";
        case is_less:
            return "less";
        case is_less_eq:
            return "less or equal";
        case is_greater:
            return "greater";
        case is_greater_eq:
            return "greater or equal";
        case is_equal:
            return "equal";
        case is_nequal:
            return "not equal";
        default:
            return "";
    }
}

std::string suffix_type_description(suffix_type t){
    switch(t){
        case no_suffix:
            return "no suffix";
        case star_power:
            return "star";
        case number_power:
            return "number power";
        default:
            return "";
    }
}

}
