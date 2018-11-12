#ifndef GAME_ITEMS
#define GAME_ITEMS

#include<map>
#include<set>
#include<vector>
#include<ostream>
#include<functional>

#include"macro_bank.hpp"
#include"token.hpp"
#include"message.hpp"
#include"slice_iterator.hpp"
#include"declarations.hpp"
#include"parsed_game.hpp"
#include"parser_helpers.hpp"
#include"sum.hpp"

namespace rbg_parser{

class unchecked_graph;
class graph;
class graph_builder;

class game_items{
        macro_bank macros;
        slice* board_segment;
        slice* players_segment;
        slice* variables_segment;
        slice* pieces_segment;
        slice* rules_segment;
        uint next_item_context_order;
        game_items(void)noexcept;

        uint input_macro(const std::vector<token>& input,uint current_token,messages_container& msg);
        uint input_board(const std::vector<token>& input,uint current_token,messages_container& msg);
        uint input_players(const std::vector<token>& input,uint current_token,messages_container& msg);
        uint input_variables(const std::vector<token>& input,uint current_token,messages_container& msg);
        uint input_pieces(const std::vector<token>& input,uint current_token,messages_container& msg);
        uint input_rules(const std::vector<token>& input,uint current_token,messages_container& msg);
        uint input_slice(
            const std::vector<token>& input,
            uint current_token,
            const std::string& segment_name,
            slice* game_items::*segment_position,
            bool should_be_nonempty,
            messages_container& msg);
        std::set<token> parse_declaration_set(
            slice* game_items::*segment_position,
            const std::string& name,
            messages_container& msg)const;
        std::map<token, uint> parse_bounded_declaration_set(
            slice* game_items::*segment_position,
            const std::string& name,
            messages_container& msg)const;
        declarations parse_declarations(messages_container& msg)const;
        std::vector<std::function<parser_result<std::unique_ptr<graph_builder>>(declarations&, slice_iterator&, messages_container&)>>
            prepare_graph_builders(void)const;
        std::unique_ptr<graph_builder> parse_graph(declarations& decl, messages_container& msg)const;
        std::unique_ptr<game_move> parse_moves(const declarations& decl, slice* game_items::*segment_position, const std::string& name, messages_container& msg)const;
    public:
        game_items(const game_items&)=delete;
        game_items(game_items&& src)noexcept;
        game_items& operator=(const game_items&)=delete;
        game_items& operator=(game_items&& src)noexcept;
        ~game_items(void)noexcept;

        friend game_items input_tokens(const std::vector<token>& input,messages_container& msg);
        parsed_game parse_game(messages_container& msg)const;
};

uint reach_end_of_directive(const std::vector<token>& input,uint current_token);
uint parse_arguments(const std::vector<token>& input,uint current_token,std::vector<token>& args,messages_container& msg);
game_items input_tokens(const std::vector<token>& input,messages_container& msg);
void print_spaces(std::ostream& out, uint n);

}

#endif
