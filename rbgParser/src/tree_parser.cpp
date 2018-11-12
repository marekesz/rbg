#include"tree_parser.hpp"
#include"slice_iterator.hpp"
#include"message.hpp"
#include"token.hpp"
#include"internal_node.hpp"
#include"bracketed_expression.hpp"
#include"typing_machine.hpp"
#include<cassert>

namespace rbg_parser{

tree_parser::tree_parser(operator_info fallback_operator, const typing_machine& t):
leaf_parsers(),
suffix_parsers(),
operators(),
fallback_operator(fallback_operator),
t(t),
brackets(){
}

void tree_parser::add_leaf_parser(parser_result<std::unique_ptr<expression>>(*p)(slice_iterator& it, messages_container& msg)){
    leaf_parsers.push_back(p);
}

void tree_parser::add_suffix_parser(parser_result<suffix>(*p)(slice_iterator& it, messages_container& msg)){
    suffix_parsers.push_back(p);
}

void tree_parser::add_operator(token_type representation, operator_info meaning){
    operators.insert(std::make_pair(representation, meaning));
}

void tree_parser::add_bracket(token_type opening, bracket_info&& br){
    brackets.insert(std::make_pair(opening, std::move(br)));
}

suffix tree_parser::parse_suffix(slice_iterator& it, messages_container& msg){
    for(const auto& el: suffix_parsers){
        auto result = el(it, msg);
        if(result.is_success())
            return result.move_value();
    }
    return {no_suffix,0};
}

parser_result<std::unique_ptr<expression>> tree_parser::parse_infix(slice_iterator& it, messages_container& msg){
    for(const auto& el: leaf_parsers){
        auto beginning = it;
        auto content = el(it,msg);
        if(content.is_success()){
            content.get_value()->type(t, msg);
            if(t.can_have_suffix(content.get_value()->get_type())){
                auto su = parse_suffix(it, msg);
                if(su.t != no_suffix){
                    bracketed_expression* naked_result = new bracketed_expression(std::move(beginning), content.move_value(), no_brackets);
                    naked_result->s = su;
                    std::unique_ptr<expression> result(naked_result);
                    return success(std::move(result));
                }
                else
                    return content;
            }
            else
                return content;
        }
    }
    return parse_bracketed_expression(it, msg);
}

parser_result<std::unique_ptr<expression>> tree_parser::parse_bracketed_expression(slice_iterator& it, messages_container& msg){
    if(not it.has_value())
        return failure<std::unique_ptr<expression>>();
    auto br = it.current(msg).get_type();
    auto search_result = brackets.find(br);
    if(search_result == brackets.end())
        return failure<std::unique_ptr<expression>>();
    auto beginning = it;
    const bracket_info& info = search_result->second;
    it.next(msg);
    auto inside_expression = parse_naked_expression(it, msg);
    if(it.current(msg).get_type() != info.ending)
        throw msg.build_message(it.create_call_stack("Expected corresponding \'"+info.expected_string+"\', encountered \'"+it.current(msg).to_string()+"\'"));
    it.next(msg);
    inside_expression->type(t, msg);
    bool can_add_suffix = t.can_have_suffix(inside_expression->get_type());
    bracketed_expression* naked_result = new bracketed_expression(std::move(beginning), std::move(inside_expression), info.meaning);
    if(can_add_suffix){
        auto su = parse_suffix(it,msg);
        naked_result->s = su;
    }
    std::unique_ptr<expression> result(naked_result);
    return success(std::move(result));
}

std::pair<bool,operator_info> tree_parser::parse_operator(slice_iterator& it, messages_container& msg){
    if(not it.has_value())
        return std::make_pair(false, fallback_operator);
    auto next_token_type = it.current(msg).get_type();
    auto search_result = operators.find(next_token_type);
    if(search_result == operators.end())
        return std::make_pair(false, fallback_operator);
    else{
        it.next(msg);
        return std::make_pair(true, search_result->second);
    }
}

bool tree_parser::append_one_element(
    std::vector<std::tuple<operator_info, slice_iterator, std::vector<std::unique_ptr<expression>>>>& expressions_stack,
    slice_iterator& it, messages_container& msg){
    auto next_operator = parse_operator(it, msg);
    if(next_is_prefixable(it, msg)){
        prepare_for_prefixable(expressions_stack, it, msg);
        return true;
    }
    auto next_element = parse_infix(it, msg);
    if(not next_element.is_success()){
        if(next_operator.first)
            throw msg.build_message(it.create_call_stack("Expected some operand, encountered \'"+it.current(msg).to_string()+"\'"));
        else
            return false;
    }
    if(std::get<0>(expressions_stack.back()).op == no_operator){
        std::get<0>(expressions_stack.back()) = next_operator.second;
        std::get<2>(expressions_stack.back()).push_back(next_element.move_value());
        return true;
    }
    end_operators_with_higher_priority(expressions_stack, next_operator.second.priority);
    if(std::get<0>(expressions_stack.back()).priority > next_operator.second.priority){
        assert(expressions_stack.size() == 1);
        auto beginning = std::get<1>(expressions_stack.back());
        auto expr = std::unique_ptr<expression>(new internal_node(
            std::move(std::get<1>(expressions_stack.back())),
            std::move(std::get<2>(expressions_stack.back())),
            std::get<0>(expressions_stack.back()).op));
        expressions_stack.pop_back();
        std::vector<std::unique_ptr<expression>> actual_top;
        actual_top.push_back(std::move(expr));
        actual_top.push_back(next_element.move_value());
        expressions_stack.push_back(std::make_tuple(next_operator.second, std::move(beginning), std::move(actual_top)));
    }
    else if(std::get<0>(expressions_stack.back()).priority == next_operator.second.priority){
        assert(std::get<0>(expressions_stack.back()).op == next_operator.second.op); // we don't support equal priority operators yet
        std::get<2>(expressions_stack.back()).push_back(next_element.move_value());
    }
    else{
        auto first_operand = std::move(std::get<2>(expressions_stack.back()).back());
        auto beginning = first_operand->get_beginning();
        std::get<2>(expressions_stack.back()).pop_back();
        std::vector<std::unique_ptr<expression>> actual_top;
        actual_top.push_back(std::move(first_operand));
        actual_top.push_back(next_element.move_value());
        expressions_stack.push_back(std::make_tuple(next_operator.second, std::move(beginning), std::move(actual_top)));
    }
    return true;
}

std::unique_ptr<expression> tree_parser::flatten_stack(
    std::vector<std::tuple<operator_info, slice_iterator, std::vector<std::unique_ptr<expression>>>>& expressions_stack){
    end_operators_with_higher_priority(expressions_stack, 0);
    if(std::get<2>(expressions_stack.back()).size() == 1)
        return std::move(std::get<2>(expressions_stack.back())[0]);
    else
        return std::unique_ptr<expression>(new internal_node(
            std::move(std::get<1>(expressions_stack.back())),
            std::move(std::get<2>(expressions_stack.back())),
            std::get<0>(expressions_stack.back()).op));
}

bool tree_parser::next_is_prefixable(slice_iterator& it, messages_container& msg){
    if(not it.has_value())
        return false;
    auto next_token_type = it.current(msg).get_type();
    auto found_info = operators.find(next_token_type);
    if(found_info == operators.end())
        return false;
    return found_info->second.prefixable;
}

void tree_parser::prepare_for_prefixable(
    std::vector<std::tuple<operator_info, slice_iterator, std::vector<std::unique_ptr<expression>>>>& expressions_stack,
    slice_iterator& it, messages_container& msg){
    auto next_token_type = it.current(msg).get_type();
    auto beginning = it;
    auto found_info = operators.find(next_token_type)->second;
    assert(found_info.prefixable);
    end_operators_with_higher_priority(expressions_stack, found_info.priority);
    expressions_stack.push_back(std::make_tuple(found_info, std::move(beginning), std::vector<std::unique_ptr<expression>>()));
}

void tree_parser::end_operators_with_higher_priority(
    std::vector<std::tuple<operator_info, slice_iterator, std::vector<std::unique_ptr<expression>>>>& expressions_stack,
    uint priority){
    while(expressions_stack.size() > 1 && std::get<0>(expressions_stack.back()).priority > priority){
        auto expr = std::unique_ptr<expression>(new internal_node(
            std::move(std::get<1>(expressions_stack.back())),
            std::move(std::get<2>(expressions_stack.back())),
            std::get<0>(expressions_stack.back()).op));
        expressions_stack.pop_back();
        std::get<2>(expressions_stack.back()).push_back(std::move(expr));
    }
}

std::unique_ptr<expression> tree_parser::parse_naked_expression(slice_iterator& it, messages_container& msg){
    parsing_context_string_guard g(&it, "Unexpected end of input while parsing expression");
    std::vector<std::tuple<operator_info, slice_iterator, std::vector<std::unique_ptr<expression>>>> expressions_stack;
    auto beginning = it;
    if(next_is_prefixable(it, msg))
        prepare_for_prefixable(expressions_stack, it, msg);
    else{
        auto first_expr = parse_infix(it, msg);
        if(not first_expr.is_success())
            return std::unique_ptr<expression>(new internal_node(std::move(beginning), {}, no_operator));
        expressions_stack.push_back(std::make_tuple(
            operator_info{no_operator, 0, false},
            first_expr.get_value()->get_beginning(),
            std::vector<std::unique_ptr<expression>>()));
        std::get<2>(expressions_stack.back()).push_back(first_expr.move_value());
    }
    while(append_one_element(expressions_stack, it, msg));
    return flatten_stack(expressions_stack);
}

}
