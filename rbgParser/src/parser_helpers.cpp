#include"parser_helpers.hpp"
#include"types.hpp"
#include"declarations.hpp"

namespace rbg_parser{


parser_result<std::vector<token>> parse_sequence_with_holes(
slice_iterator& it,
const std::set<token>& verification_set,
messages_container& msg){
    std::vector<token> result;
    if(!it.has_value())
        return success(std::move(result));
    while(it.has_value() and (it.current(msg).get_type() == comma or it.current(msg).get_type() == identifier)){
        if(not result.empty())
            it.next(msg);
        if(it.current(msg).get_type() == identifier){
            if(verification_set.find(it.current(msg)) != verification_set.end())
                result.push_back(it.current(msg));
            else
                throw msg.build_message(it.create_call_stack("Identifier \'"+it.current(msg).to_string()+"\' was not declared in respective segment"));
            it.next(msg);
        }
        else
            result.push_back(token());
    }
    return success(std::move(result));
}

parser_result<std::vector<token>> parse_sequence(
slice_iterator& it,
const std::string& purpose_name,
const std::set<token>& verification_set,
bool should_verify,
messages_container& msg){
    parsing_context_string_guard g(&it, "Unexpected end of input while parsing "+purpose_name);
    std::vector<token> result;
    if(!it.has_value())
        return success(std::move(result));
    bool should_meet_comma = false;
    while(it.has_value() && (it.current(msg).get_type() == comma || it.current(msg).get_type() == identifier)){
        if(it.current(msg).get_type() == comma)
            should_meet_comma = false;
        else if(it.current(msg).get_type() == identifier){
            if(should_meet_comma)
                throw msg.build_message(it.create_call_stack("Two identifiers should be separated by at least one comma"));
            else if(!should_verify || verification_set.find(it.current(msg)) != verification_set.end())
                result.push_back(it.current(msg));
            else
                throw msg.build_message(it.create_call_stack("Identifier \'"+it.current(msg).to_string()+"\' was not declared in respective segment"));
            should_meet_comma = true;
        }
        it.next(msg);
    }
    return success(std::move(result));
}

parser_result<std::map<token, uint>> parse_bounded_sequence(
    slice_iterator& it,
    const std::string& purpose_name,
    messages_container& msg){
    parsing_context_string_guard g(&it, "Unexpected end of input while parsing "+purpose_name);
    std::map<token, uint> result;
    if(!it.has_value())
        return success(std::move(result));
    bool should_meet_comma = false;
    while(it.has_value() && (it.current(msg).get_type() == comma || it.current(msg).get_type() == identifier)){
        if(it.current(msg).get_type() == comma)
            should_meet_comma = false;
        else if(it.current(msg).get_type() == identifier){
            if(should_meet_comma)
                throw msg.build_message(it.create_call_stack("Two identifiers should be separated by at least one comma"));
            auto name = it.current(msg);
            it.next(msg);
            if(it.current(msg).get_type() != left_round_bracket)
                throw msg.build_message(it.create_call_stack("Expected \'(\', encountered \'"+it.current(msg).to_string()+"\'"));
            it.next(msg);
            if(it.current(msg).get_type() != number || it.current(msg).get_value() <= 0)
                throw msg.build_message(it.create_call_stack("Expected positive integer, encountered \'"+it.current(msg).to_string()+"\'"));
            uint variable_bound = it.current(msg).get_value();
            it.next(msg);
            if(it.current(msg).get_type() != right_round_bracket)
                throw msg.build_message(it.create_call_stack("Expected \')\', encountered \'"+it.current(msg).to_string()+"\'"));
            should_meet_comma = true;
            result.insert(std::make_pair(std::move(name), variable_bound));
        }
        it.next(msg);
    }
    return success(std::move(result));
}

token parse_edge_name(declarations& decl, slice_iterator& it, messages_container& msg){
    if(it.current(msg).get_type() != identifier)
        throw msg.build_message(it.create_call_stack("Expected edge label, encountered \'"+it.current(msg).to_string()+"\'"));
    auto label_name = it.current(msg);
    if(decl.get_legal_pieces().find(label_name) != decl.get_legal_pieces().end())
        throw msg.build_message(it.create_call_stack("Edge label \'"+it.current(msg).to_string()+"\' was already declared as piece"));
    if(decl.get_legal_players().find(label_name) != decl.get_legal_players().end())
        throw msg.build_message(it.create_call_stack("Edge label \'"+it.current(msg).to_string()+"\' was already declared as player"));
    if(decl.get_legal_variables().find(label_name) != decl.get_legal_variables().end())
        throw msg.build_message(it.create_call_stack("Edge label \'"+it.current(msg).to_string()+"\' was already declared as variable"));
    if(decl.get_legal_edges().find(label_name) != decl.get_legal_edges().end())
        throw msg.build_message(it.create_call_stack("Edge label \'"+it.current(msg).to_string()+"\' was already declared"));
    decl.add_edge_label(label_name);
    it.next(msg);
    return label_name;
}

}
