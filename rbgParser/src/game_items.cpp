#include<cassert>

#include"game_items.hpp"
#include"types.hpp"
#include"parser_helpers.hpp"
#include"rules_parser.hpp"
#include"tree_utils.hpp"
#include"typing_machine.hpp"
#include"graph.hpp"
#include"graph_builder.hpp"
#include"unchecked_graph.hpp"
#include"rectangle2D.hpp"
#include"hexagon2D.hpp"
#include"cuboid3D.hpp"

namespace rbg_parser{

game_items::game_items(void)noexcept:
macros(),
board_segment(nullptr),
players_segment(nullptr),
variables_segment(nullptr),
pieces_segment(nullptr),
rules_segment(nullptr),
next_item_context_order(0){
}

game_items::game_items(game_items&& src)noexcept:
macros(std::move(src.macros)),
board_segment(src.board_segment),
players_segment(src.players_segment),
variables_segment(src.variables_segment),
pieces_segment(src.pieces_segment),
rules_segment(src.rules_segment),
next_item_context_order(src.next_item_context_order){
    src.board_segment = src.players_segment = src.variables_segment = src.pieces_segment = src.rules_segment = nullptr;
}

game_items& game_items::operator=(game_items&& src)noexcept{
    if(this != &src){
        macro_bank temp(std::move(src.macros));
        src.macros = std::move(macros);
        macros = std::move(temp);
        std::swap(board_segment,src.board_segment);
        std::swap(players_segment,src.players_segment);
        std::swap(variables_segment,src.variables_segment);
        std::swap(pieces_segment,src.pieces_segment);
        std::swap(rules_segment,src.rules_segment);
        next_item_context_order = src.next_item_context_order;
    }
    return *this;
}

game_items::~game_items(void)noexcept{
    delete board_segment;
    delete players_segment;
    delete variables_segment;
    delete pieces_segment;
    delete rules_segment;
}

uint game_items::input_macro(const std::vector<token>& input,uint current_token,messages_container& msg){
    if(current_token >= input.size() || input[current_token].get_type() == hash)
        throw msg.build_message(input[current_token-1].get_position(),"Empty macro directive");
    if(input[current_token].get_type() != identifier)
        throw msg.build_message(input[current_token].get_position(),"Expected identifier, encountered \'"+input[current_token].to_string()+"\'");
    token name = input[current_token];
    std::vector<token> args;
    current_token = parse_arguments(input,current_token+1,args,msg);
    if((args.size()>0 && macros.is_zero_arity_macro(name,next_item_context_order)) || (args.size()==0 && macros.is_non_zero_arity_macro(name,next_item_context_order)))
        throw msg.build_message(name.get_position(),"Cannot define two macros with the same name with zero and non zero arities");
    if(macros.is_macro_name_and_arity(name,args.size(),next_item_context_order))
        msg.add_message(name.get_position(),"Macro \'"+name.to_string()+"\' with arity "+std::to_string(args.size())+" already defined, overwriting previous definition");
    if(current_token >= input.size() || input[current_token].get_type() == hash)
        throw msg.build_message(input[current_token-1].get_position(),"Unexpected end of macro definition");
    if(input[current_token].get_type() != equal)
        throw msg.build_message(input[current_token].get_position(),"Expected \'=\' token, encountered \'"+input[current_token].to_string()+"\'");
    uint begin = current_token+1;
    uint end = current_token = reach_end_of_directive(input,begin);
    macro m = macro(std::move(args),&input,begin,end,next_item_context_order++);
    if(m.is_empty())
        msg.add_message(name.get_position(),"Macro \'"+name.to_string()+"\' doesn't have body");
    macros.add_macro(std::move(name),std::move(m));
    return current_token;
}

uint game_items::input_slice(
const std::vector<token>& input,
uint current_token,
const std::string& segment_name,
slice* game_items::*segment_position,
bool should_be_nonempty,
messages_container& msg){
    if(current_token >= input.size())
        throw msg.build_message(input[current_token-1].get_position(),"Unexpected end of \'"+segment_name+"\' segment");
    if(input[current_token].get_type() != equal)
        throw msg.build_message(input[current_token].get_position(),"Expected \'=\' after \'"+segment_name+"\' segment name, encountered \'"+input[current_token].to_string()+"\'");
    ++current_token;
    uint begin = current_token;
    uint end = current_token = reach_end_of_directive(input,current_token);
    slice* s = new slice(&input,begin,end,next_item_context_order++);
    if(s->is_empty() && should_be_nonempty)
        msg.add_message(input[begin-2].get_position(),"Empty \'"+segment_name+"\' directive");
    if(this->*segment_position)
        msg.add_message(input[begin-2].get_position(),"Another \'"+segment_name+"\' declared; discarding previous");
    delete (this->*segment_position);
    this->*segment_position = s;
    return end;
}

uint game_items::input_board(const std::vector<token>& input,uint current_token,messages_container& msg){
    return input_slice(input, current_token, "board", &game_items::board_segment,true,msg);
}

uint game_items::input_players(const std::vector<token>& input,uint current_token,messages_container& msg){
    return input_slice(input, current_token, "players", &game_items::players_segment,true,msg);
}

uint game_items::input_variables(const std::vector<token>& input,uint current_token,messages_container& msg){
    return input_slice(input, current_token, "variables", &game_items::variables_segment,false,msg);
}

uint game_items::input_pieces(const std::vector<token>& input,uint current_token,messages_container& msg){
    return input_slice(input, current_token, "pieces", &game_items::pieces_segment,true,msg);
}

uint game_items::input_rules(const std::vector<token>& input,uint current_token,messages_container& msg){
    return input_slice(input, current_token, "rules", &game_items::rules_segment,true,msg);
}

uint reach_end_of_directive(const std::vector<token>& input,uint current_token){
    while(current_token < input.size() && input[current_token].get_type() != hash)
        ++current_token;
    return current_token;
}

uint parse_arguments(const std::vector<token>& input,uint current_token,std::vector<token>& args,messages_container& msg){
    if(input[current_token].get_type() != left_round_bracket || current_token >= input.size())
        return current_token;
    while(true){
        ++current_token;
        if(current_token >= input.size() || input[current_token].get_type() == hash)
            throw msg.build_message(input[current_token-1].get_position(),"Unexpected end of macro arguments list");
        if(input[current_token].get_type() != identifier)
            throw msg.build_message(input[current_token].get_position(),"Expected identifier, encountered \'"+input[current_token].to_string()+"\'");
        args.push_back(input[current_token++]);
        if(current_token >= input.size() || input[current_token].get_type() == hash)
            throw msg.build_message(input[current_token-1].get_position(),"Unexpected end of macro arguments list");
        if(input[current_token].get_type() == right_round_bracket)
            return current_token+1;
        if(input[current_token].get_type() != semicolon)
            throw msg.build_message(input[current_token].get_position(),"Expected \';\' or \'(\' token, encounered \'"+input[current_token].to_string()+"\'");
    }
}

game_items input_tokens(const std::vector<token>& input,messages_container& msg){
    game_items result;
    if(input.empty())
        throw msg.build_message("No tokens in file");
    uint current_token = 0;
    if(input[current_token].get_type() != hash)
        throw msg.build_message(input[current_token].get_position(),"Expected \'#\' token at beginning of file, encountered \'"+input[current_token].to_string()+"\'");
    while(current_token < input.size()){
        ++current_token;
        if(current_token < input.size()){
            switch(input[current_token].get_type()){
            case board:
                current_token = result.input_board(input,current_token+1,msg);
                break;
            case players:
                current_token = result.input_players(input,current_token+1,msg);
                break;
            case variables:
                current_token = result.input_variables(input,current_token+1,msg);
                break;
            case pieces:
                current_token = result.input_pieces(input,current_token+1,msg);
                break;
            case rules:
                current_token = result.input_rules(input,current_token+1,msg);
                break;
            case identifier:
                current_token = result.input_macro(input,current_token,msg);
                break;
            default:
                throw msg.build_message(input[current_token].get_position(),"Expected \'game\', \'board\', \'players\', \'variables\', \'pieces\' or \'rules\' token or identifier, encountered \'"+input[current_token].to_string()+"\'");
            }
        }
    }
    if(result.board_segment == nullptr)
        throw msg.build_message("No \'board\' directive");
    if(result.players_segment == nullptr)
        throw msg.build_message("No \'players\' directive");
    if(result.variables_segment == nullptr)
        throw msg.build_message("No \'variables\' directive");
    if(result.pieces_segment == nullptr)
        throw msg.build_message("No \'pieces\' directive");
    if(result.rules_segment == nullptr)
        throw msg.build_message("No \'rules\' directive");
    return result;
}

std::set<token> game_items::parse_declaration_set(
    slice* game_items::*segment_position,
    const std::string& name,
    messages_container& msg)const{
    slice_iterator it(*(this->*segment_position),&macros);
    parsing_context_string_guard g(&it, "Unexpected end of input while parsing \'"+name+"\' segment");
    it.next(msg);
    auto parsing_result = parse_sequence(it,name+" list",std::set<token>(),false,msg);
    if(!parsing_result.is_success())
        throw msg.build_message("Expected comma-separated list of identifiers in \'"+name+"\' segment");
    auto sequence = parsing_result.move_value();
    std::set<token> result(sequence.begin(),sequence.end());
    if(it.has_value())
        msg.add_message(it.create_call_stack("Unexpected tokens at the end of \'"+name+"\' segment"));
    return result;
}

std::map<token, uint> game_items::parse_bounded_declaration_set(
    slice* game_items::*segment_position,
    const std::string& name,
    messages_container& msg)const{
    slice_iterator it(*(this->*segment_position),&macros);
    parsing_context_string_guard g(&it, "Unexpected end of input while parsing \'"+name+"\' segment");
    it.next(msg);
    auto parsing_result = parse_bounded_sequence(it,name+" list",msg);
    if(not parsing_result.is_success())
        throw msg.build_message("Expected comma-separated list of identifiers in \'"+name+"\' segment");
    auto result = parsing_result.move_value();
    if(it.has_value())
        msg.add_message(it.create_call_stack("Unexpected tokens at the end of \'"+name+"\' segment"));
    return result;
}

declarations game_items::parse_declarations(messages_container& msg)const{
    declarations result(
        parse_bounded_declaration_set(&game_items::players_segment,"players",msg),
        parse_declaration_set(&game_items::pieces_segment,"pieces",msg),
        parse_bounded_declaration_set(&game_items::variables_segment,"variables",msg)
    );
    const auto& legal_pieces = result.get_legal_pieces();
    const auto& legal_players = result.get_legal_players();
    const auto& legal_variables = result.get_legal_variables();
    for(const auto& el: legal_pieces)
        if(legal_players.find(el) != legal_players.end())
            throw msg.build_message("Identifier \'"+el.to_string()+"\' is found in both \'pieces\' and \'players\' declarations");
    for(const auto& el: legal_pieces)
        if(legal_variables.find(el) != legal_variables.end())
            throw msg.build_message("Identifier \'"+el.to_string()+"\' is found in both \'pieces\' and \'variables\' declarations");
    for(const auto& el: legal_players)
        if(legal_variables.find(el.first) != legal_variables.end())
            throw msg.build_message("Identifier \'"+el.first.to_string()+"\' is found in both \'players\' and \'variables\' declarations");
    return result;
}

std::vector<std::function<parser_result<std::unique_ptr<graph_builder>>(declarations&, slice_iterator&, messages_container&)>>
    game_items::prepare_graph_builders(void)const{
    std::vector<std::function<parser_result<std::unique_ptr<graph_builder>>(declarations&, slice_iterator&, messages_container&)>> result;
    result.push_back(parse_hexagon2D);
    result.push_back(parse_rectangle2D);
    result.push_back(parse_cuboid3D);
    result.push_back(parse_unchecked_graph); // must be last
    return std::move(result);
}

std::unique_ptr<graph_builder> game_items::parse_graph(declarations& decl, messages_container& msg)const{
    slice_iterator it(*board_segment,&macros);
    parsing_context_string_guard g(&it, "Unexpected end of input while parsing \'board\' segment");
    it.next(msg);
    auto graph_builders = prepare_graph_builders();
    for(const auto& el: graph_builders){
        auto parsing_result = el(decl, it, msg);
        if(parsing_result.is_success())
            return parsing_result.move_value();
    }
    throw msg.build_message(it.create_call_stack("Couldn't interpret this as board game"));
}

std::unique_ptr<game_move> game_items::parse_moves(const declarations& decl, slice* game_items::*segment_position, const std::string& name, messages_container& msg)const{
    slice_iterator it(*(this->*segment_position),&macros);
    it.next(msg);
    auto typer = prepare_types_for_rbg(decl);
    auto res = parse_rules(it, typer, msg);
    if(it.has_value())
        msg.add_message(it.create_call_stack("Unexpected tokens at the end of \'"+name+"\' segment"));
    res->type(typer, msg);
    if(not is_subtype(gmove, res->get_type()))
        throw msg.build_message("Segment \'"+name+"\' is of type \'"+expression_type_description(res->get_type())+"\' (should be move  expression)");
    return res->get_game_move()->simplify()->flatten();
}

parsed_game game_items::parse_game(messages_container& msg)const{
    declarations decl = parse_declarations(msg);
    auto parsed_graph_builder = parse_graph(decl,msg);
    graph g = parsed_graph_builder->build_graph(msg);
    std::unique_ptr<game_move> moves = parse_moves(decl,&game_items::rules_segment,"rules",msg);
    return parsed_game(
        std::move(decl),
        std::move(g),
        std::move(moves)
    );
}

}
