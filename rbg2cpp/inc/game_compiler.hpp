#ifndef GAME_COMPILER
#define GAME_COMPILER

#include<string>
#include<map>
#include<vector>

#include"cpp_container.hpp"
#include"types.hpp"
#include"automaton.hpp"
#include"shift_table.hpp"
#include"precomputed_pattern.hpp"

class compiler_options;

namespace rbg_parser{
    class token;
    class parsed_game;
}

class game_compiler{
        cpp_container output;
        const compiler_options& opts;
        std::string name;
        std::map<rbg_parser::token, uint> pieces_to_id;
        std::map<rbg_parser::token, uint> edges_to_id;
        std::map<rbg_parser::token, uint> variables_to_id;
        automaton game_automaton;
        std::vector<automaton> pattern_automata;
        std::vector<shift_table> shift_tables;
        std::vector<precomputed_pattern> precomputed_patterns;
        const rbg_parser::parsed_game& input;
        game_compiler(void)=delete;
        void generate_board_cells_decoder(void);
        void generate_pieces_decoder(void);
        void generate_variables_decoder(void);
        void fill_edges_map(void);
        std::string numbers_to_array(const std::vector<int>& numbers)const;
        void generate_board_structure(void);
        void generate_appliers_lists(void);
        void generate_move_class(void);
        void generate_game_state_class(void);
        void generate_initial_pieces(void);
        void generate_variables_bounds(void);
        void generate_initial_variables(void);
        void generate_state_getters(void);
        void generate_game_parameters(void);
        void build_game_automaton(void);
        void generate_states_iterator(void);
        void generate_iterator_helper_structures(void);
        void generate_resettable_bitarray(void);
        void generate_resettable_bitarray_stack(void);
        void generate_main_next_getters(void);
        void generate_pattern_evaluator(uint pattern_index);
        void print_all_shift_tables(void);
    public:
        game_compiler(const rbg_parser::parsed_game& input, const compiler_options& opts);
        game_compiler(const game_compiler&)=delete;
        game_compiler(game_compiler&&)=delete;
        game_compiler& operator=(const game_compiler&)=delete;
        game_compiler& operator=(game_compiler&&)=delete;
        ~game_compiler(void)=default;
        const cpp_container& compile(void);
};

#endif
