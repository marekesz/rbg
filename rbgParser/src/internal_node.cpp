#include"internal_node.hpp"
#include"typing_machine.hpp"
#include"arithmetic_expression.hpp"
#include"arithmetic_operation.hpp"
#include"sum.hpp"
#include"concatenation.hpp"
#include"arithmetic_comparison.hpp"

namespace rbg_parser{

internal_node::internal_node(
    slice_iterator&& beginning_position,
    std::vector<std::unique_ptr<expression>>&& elements,
    operator_type op):
expression(std::move(beginning_position)),
elements(std::move(elements)),
op(op),
t(not_typed_yet){
}

expression_type internal_node::get_type(void)const{
    return t;
}

void internal_node::type(const typing_machine& m, messages_container& msg){
    if(t != not_typed_yet)
        return;
    std::vector<expression_type> elements_types;
    for(const auto& el: elements){
        el->type(m, msg);
        elements_types.push_back(el->get_type());
    }
    auto result = m.evaluate_operator_sequence(op, elements_types);
    if(result == error_type){
        std::string error_message = operator_type_description(op)+" cannot be applied to: ";
        if(not elements_types.empty())
            error_message += expression_type_description(elements_types[0]);
        for(uint i=1;i<elements_types.size();++i)
            error_message += ", "+expression_type_description(elements_types[i]);
        throw msg.build_message(beginning_position.create_call_stack(error_message));
    }
    t = result;
}

std::vector<token> internal_node::get_identifiers_sequence(void)const{
    assert(t == pieces_sequence);
    std::vector<token> result;
    for(const auto& el: elements)
        result.push_back(el->get_identifier());
    return std::move(result);
}

std::vector<std::pair<token, std::unique_ptr<arithmetic_expression>>> internal_node::get_assignments_sequence(void)const{
    switch(t){
        case assignments_sequence:
        {
            std::vector<std::pair<token, std::unique_ptr<arithmetic_expression>>> result;
            for(const auto& el: elements)
                result.push_back(el->get_assignment());
            return std::move(result);
        }
        case single_assignment:
        {
            std::vector<std::pair<token, std::unique_ptr<arithmetic_expression>>> result;
            result.push_back(get_assignment());
            return std::move(result);
        }
        default:
            assert(false);
    }
}

std::pair<token, std::unique_ptr<arithmetic_expression>> internal_node::get_assignment(void)const{
    assert(t == single_assignment);
    return std::make_pair(elements[0]->get_identifier(), elements[1]->get_arithmetic_expression());
}

std::unique_ptr<arithmetic_expression> internal_node::get_arithmetic_expression(void)const{
    assert(t == arithmetics);
    std::vector<std::unique_ptr<arithmetic_expression>> result;
    for(const auto& el: elements)
        result.push_back(el->get_arithmetic_expression());
    switch(op){
        case add:
            return std::unique_ptr<arithmetic_expression>(new arithmetic_operation(std::move(result), addition));
        case subtract:
            return std::unique_ptr<arithmetic_expression>(new arithmetic_operation(std::move(result), subtraction));
        case multiply:
            return std::unique_ptr<arithmetic_expression>(new arithmetic_operation(std::move(result), multiplication));
        case divide:
            return std::unique_ptr<arithmetic_expression>(new arithmetic_operation(std::move(result), division));
        default:
            assert(false);
    }
}

std::unique_ptr<game_move> internal_node::get_game_move(void)const{
    assert(t == gmove || t == integer_comparison);
    if(t == integer_comparison){
        kind kind_of_comparison = eq;
        switch(op){
            case is_equal:
                kind_of_comparison = eq;
                break;
            case is_nequal:
                kind_of_comparison = neq;
                break;
            case is_less:
                kind_of_comparison = le;
                break;
            case is_less_eq:
                kind_of_comparison = leq;
                break;
            case is_greater:
                kind_of_comparison = ge;
                break;
            case is_greater_eq:
                kind_of_comparison = geq;
                break;
            default:
                break;
        }
        return std::unique_ptr<game_move>(new arithmetic_comparison(
            elements[0]->get_arithmetic_expression(),
            kind_of_comparison,
            elements[1]->get_arithmetic_expression()));
    }
    std::vector<std::unique_ptr<game_move>> result;
    for(const auto& el: elements)
        result.push_back(el->get_game_move());
    switch(op){
        case add:
        case no_operator:
            return std::unique_ptr<game_move>(new sum(std::move(result)));
        case concatenate:
            return std::unique_ptr<game_move>(new concatenation(std::move(result)));
        default:
            assert(false);
    }
}

}
