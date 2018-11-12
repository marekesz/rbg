#include"typing_machine.hpp"
#include"declarations.hpp"

namespace rbg_parser{

typing_machine::typing_machine(const declarations& decls):
decls(decls){
}

expression_type typing_machine::evaluate_identifier(const token& t)const{
    const auto& pieces = decls.get_legal_pieces();
    if(pieces.find(t) != pieces.end())
        return piece_name;
    const auto& players = decls.get_legal_players();
    if(players.find(t) != players.end())
        return player_name;
    const auto& vars = decls.get_legal_variables();
    if(vars.find(t) != vars.end())
        return variable;
    const auto& edges = decls.get_legal_edges();
    if(edges.find(t) != edges.end())
        return shift_move;
    return error_type;
}

bool typing_machine::can_be_switch(const token& t)const{
    const auto& players = decls.get_legal_players();
    return players.find(t) != players.end();
}

void typing_machine::add_operator_interpretation(
    operator_type t,
    bool arity_matters,
    std::vector<expression_type>&& types_to_match,
    expression_type result){
    possible_operator_interpretation op_int{arity_matters, std::move(types_to_match), result};
    auto it = operator_meanings.find(t);
    if(it == operator_meanings.end())
        operator_meanings.insert(std::make_pair(t, std::vector<possible_operator_interpretation>{op_int}));
    else
        it->second.push_back(op_int);
}

void typing_machine::add_bracket_interpretation(
    bracket_type t,
    expression_type type_to_match,
    std::set<suffix_type>&& suffixes_to_match,
    expression_type result){
    possible_bracket_interpretation br_int{type_to_match, std::move(suffixes_to_match), result};
    auto it = bracket_meanings.find(t);
    if(it == bracket_meanings.end())
        bracket_meanings.insert(std::make_pair(t, std::vector<possible_bracket_interpretation>{br_int}));
    else
        it->second.push_back(br_int);
}

void typing_machine::set_as_suffixable(expression_type t){
    suffixable_types.insert(t);
}

bool typing_machine::can_have_suffix(expression_type t)const{
    for(const auto el: suffixable_types)
        if(is_subtype(el, t))
            return true;
    return false;
}

bool typing_machine::operator_types_match(const std::vector<expression_type>& elements, const possible_operator_interpretation& op_int)const{
    if(op_int.arity_matters){
        if(elements.size() != op_int.types_to_match.size())
            return false;
        for(uint i=0;i<elements.size();++i)
            if(not is_subtype(op_int.types_to_match[i], elements[i]))
                return false;
        return true;
    }
    else{
        for(const auto& el: elements)
            if(not is_subtype(op_int.types_to_match[0], el))
                return false;
        return true;
    }
}

bool typing_machine::bracket_types_match(expression_type element, suffix_type s, const possible_bracket_interpretation& br_int)const{
    if(not is_subtype(br_int.type_to_match, element))
        return false;
    return br_int.suffixes_to_match.find(s) != br_int.suffixes_to_match.end();
}

expression_type typing_machine::evaluate_operator_sequence(operator_type t, const std::vector<expression_type>& elements)const{
    auto it = operator_meanings.find(t);
    if(it == operator_meanings.end())
        return error_type;
    const auto& possible_results = it->second;
    for(const auto& el: possible_results)
        if(operator_types_match(elements, el))
            return el.resulting_type;
    return error_type;
}

expression_type typing_machine::evaluate_brackets(bracket_type t, expression_type element, suffix_type s)const{
    auto it = bracket_meanings.find(t);
    if(it == bracket_meanings.end())
        return error_type;
    const auto& possible_results = it->second;
    for(const auto& el: possible_results)
        if(bracket_types_match(element, s, el))
            return el.resulting_type;
    return error_type;
}

typing_machine prepare_types_for_rbg(const declarations& decls){
    typing_machine result(decls);

    result.set_as_suffixable(gmove);

    result.add_operator_interpretation(no_operator, true, std::vector<expression_type>{}, pieces_sequence);
    result.add_operator_interpretation(concatenate, false, std::vector<expression_type>{gmove}, gmove);
    result.add_operator_interpretation(add, false, std::vector<expression_type>{gmove}, gmove);
    result.add_operator_interpretation(add, false, std::vector<expression_type>{arithmetics}, arithmetics);
    result.add_operator_interpretation(subtract, false, std::vector<expression_type>{arithmetics}, arithmetics);
    result.add_operator_interpretation(multiply, false, std::vector<expression_type>{arithmetics}, arithmetics);
    result.add_operator_interpretation(divide, false, std::vector<expression_type>{arithmetics}, arithmetics);
    result.add_operator_interpretation(separate, false, std::vector<expression_type>{piece_name}, pieces_sequence);
    result.add_operator_interpretation(separate, false, std::vector<expression_type>{single_assignment}, assignments_sequence);
    result.add_operator_interpretation(assign, true, std::vector<expression_type>{variable, arithmetics}, single_assignment);
    result.add_operator_interpretation(is_equal, true, std::vector<expression_type>{arithmetics, arithmetics}, integer_comparison);
    result.add_operator_interpretation(is_nequal, true, std::vector<expression_type>{arithmetics, arithmetics}, integer_comparison);
    result.add_operator_interpretation(is_less, true, std::vector<expression_type>{arithmetics, arithmetics}, integer_comparison);
    result.add_operator_interpretation(is_less_eq, true, std::vector<expression_type>{arithmetics, arithmetics}, integer_comparison);
    result.add_operator_interpretation(is_greater, true, std::vector<expression_type>{arithmetics, arithmetics}, integer_comparison);
    result.add_operator_interpretation(is_greater_eq, true, std::vector<expression_type>{arithmetics, arithmetics}, integer_comparison);

    result.add_bracket_interpretation(condition_bracket, gmove, std::set<suffix_type>{no_suffix}, mcheck);
    result.add_bracket_interpretation(negated_condition_bracket, gmove, std::set<suffix_type>{no_suffix}, mcheck);
    result.add_bracket_interpretation(comparison_bracket, integer_comparison, std::set<suffix_type>{no_suffix}, arithmetic_check);
    result.add_bracket_interpretation(assignment_bracket, assignments_sequence, std::set<suffix_type>{no_suffix}, assignments_move);
    result.add_bracket_interpretation(modifier_bracket, pieces_sequence, std::set<suffix_type>{no_suffix}, offs_move);
    result.add_bracket_interpretation(standard_bracket, gmove, std::set<suffix_type>{no_suffix, star_power, number_power}, gmove);
    result.add_bracket_interpretation(no_brackets, gmove, std::set<suffix_type>{no_suffix, star_power, number_power}, gmove);
    result.add_bracket_interpretation(standard_bracket, arithmetics, std::set<suffix_type>{no_suffix}, arithmetics);
    result.add_bracket_interpretation(on_bracket, pieces_sequence, std::set<suffix_type>{no_suffix}, on_move);

    return result;
}

}
