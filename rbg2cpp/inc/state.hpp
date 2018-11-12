#ifndef STATE
#define STATE

#include<vector>
#include<set>

#include"types.hpp"
#include"edge.hpp"

class cpp_container;
class compiler_options;
class precomputed_pattern;
class actions_compiler;

namespace rbg_parser{
    class game_move;
    class graph;
}

class state{
        std::vector<edge> next_states;
        bool outgoing_edges_needed;
        bool doubly_reachable;
    public:
        void inform_about_being_appended(uint shift_value);
        void inform_about_state_deletion(uint deleted_index);
        void absorb(state&& rhs);
        void connect_with_state(uint index_in_local_register, const std::vector<label>& label_list=std::vector<label>());
        void print_transition_functions(
            uint from_state,
            cpp_container& output,
            const std::map<rbg_parser::token, uint>& pieces_to_id,
            const std::map<rbg_parser::token, uint>& edges_to_id,
            const std::map<rbg_parser::token, uint>& variables_to_id,
            const rbg_parser::declarations& decl,
            const std::vector<state>& local_register,
            const std::vector<shift_table>& shift_tables,
            const std::vector<precomputed_pattern>& precomputed_patterns,
            bool stop_after_first)const;
        void print_transition_functions_inside_pattern(
            uint from_state,
            uint pattern_index,
            cpp_container& output,
            const std::map<rbg_parser::token, uint>& pieces_to_id,
            const std::map<rbg_parser::token, uint>& edges_to_id,
            const std::map<rbg_parser::token, uint>& variables_to_id,
            const rbg_parser::declarations& decl,
            const std::vector<state>& local_register,
            const std::vector<shift_table>& shift_tables,
            const std::vector<precomputed_pattern>& precomputed_patterns)const;
        void print_outgoing_transitions(uint from_state, cpp_container& output, const std::string& functions_prefix)const;
        void notify_endpoints_about_being_reachable(std::vector<uint>& reachability, const std::vector<shift_table>& shift_tables)const;
        void mark_as_doubly_reachable(void);
        bool can_be_checked_for_visit(void)const;
        void mark_explicitly_as_transition_start(void);
        const edge& get_only_exit(void)const;
        bool is_dead_end(void)const;
        bool is_no_choicer(void)const;
        void push_next_states_to_shift_tables_dfs_stack(
            uint current_cell,
            const rbg_parser::graph& board,
            std::vector<std::pair<uint,uint>>& dfs_stack,
            const std::vector<precomputed_pattern>& pps)const;
        void print_recursive_calls_for_all_getter(uint from_state, cpp_container& output, const std::string& cell="current_cell")const;
        void print_recursive_calls_for_any_getter(uint from_state, cpp_container& output, const actions_compiler& ac, const std::string& cell="current_cell")const;
        void print_recursive_calls_for_pattern(uint from_state, cpp_container& output, const actions_compiler& ac, uint pattern_index, const std::string& cell="current_cell")const;
};

#endif
