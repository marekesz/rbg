#ifndef AUTOMATON_BUILDER
#define AUTOMATON_BUILDER

#include"abstract_dispatcher.hpp"
#include"automaton.hpp"
#include"shift_table.hpp"
#include"precomputed_pattern.hpp"
#include"types.hpp"
#include<cassert>
#include<vector>

class compiler_options;

namespace rbg_parser{
    class game_move;
    class graph;
}

class automaton_builder : public rbg_parser::abstract_dispatcher{
        automaton local_copy_automaton;
        automaton local_copy_shift_automaton;
        automaton& currently_modified_automaton;
        automaton& currently_modified_shift_automaton;
        bool has_automaton;
        bool has_shift_automaton;
        const rbg_parser::graph& board;
        std::vector<automaton>& pattern_automata;
        std::vector<shift_table>& shift_tables;
        std::vector<precomputed_pattern>& precomputed_patterns;
        std::vector<label>& current_block;
        std::vector<label>& current_shift_block;
        const compiler_options& opts;
        void handle_any_switch(const rbg_parser::game_move& m);
        void build_automaton_from_actions_so_far(void);
        void concat_automaton_to_result_so_far(automaton&& a);
        automaton_builder(
            const rbg_parser::graph& board,
            std::vector<automaton>& pattern_automata,
            std::vector<shift_table>& shift_tables,
            std::vector<precomputed_pattern>& precomputed_patterns,
            std::vector<label>& current_block,
            std::vector<label>& current_shift_block,
            automaton& upper_level_automaton,
            automaton& upper_level_shift_automaton,
            const compiler_options& opts);
        automaton_builder delegate_builder();
        shift_table gather_shift_so_far_into_table(void);
        void end_shift_automaton(void);
        bool only_shifts(void)const;
        automaton get_only_shifts_final_result(void);
        void build_shift_automaton_from_actions_so_far(void);
        void concat_shift_automaton_to_result_so_far(automaton&& a);
        bool worth_creating_shift_table(void)const;
    public:
        automaton_builder(const automaton_builder&)=delete;
        automaton_builder(automaton_builder&&)=default;
        automaton_builder& operator=(const automaton_builder&)=delete;
        automaton_builder& operator=(automaton_builder&&)=delete;
        ~automaton_builder(void)override=default;
        automaton_builder(
            const rbg_parser::graph& board,
            std::vector<automaton>& pattern_automata,
            std::vector<shift_table>& shift_tables,
            std::vector<precomputed_pattern>& precomputed_patterns,
            std::vector<label>& current_block,
            std::vector<label>& current_shift_block,
            const compiler_options& opts);
        void dispatch(const rbg_parser::sum& m)override;
        void dispatch(const rbg_parser::concatenation& m)override;
        void dispatch(const rbg_parser::star_move& m)override;
        void dispatch(const rbg_parser::shift& m)override;
        void dispatch(const rbg_parser::ons& m)override;
        void dispatch(const rbg_parser::off& m)override;
        void dispatch(const rbg_parser::assignment& m)override;
        void dispatch(const rbg_parser::player_switch& m)override;
        void dispatch(const rbg_parser::keeper_switch& m)override;
        void dispatch(const rbg_parser::move_check& m)override;
        void dispatch(const rbg_parser::arithmetic_comparison& m)override;
        void dispatch(const rbg_parser::integer_arithmetic&)override{assert(false);}
        void dispatch(const rbg_parser::variable_arithmetic&)override{assert(false);}
        void dispatch(const rbg_parser::arithmetic_operation&)override{assert(false);}
        automaton get_final_result(void);
};

#endif
