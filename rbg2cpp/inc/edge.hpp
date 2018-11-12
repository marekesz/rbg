#ifndef EDGE
#define EDGE

#include<vector>
#include<map>
#include"types.hpp"
#include"token.hpp"

class cpp_container;
class state;
class actions_compiler;
class compiler_options;
class precomputed_pattern;
class shift_table;

namespace rbg_parser{
    class game_move;
    class declarations;
    class graph;
}

enum label_kind{
    action,
    positive_pattern,
    negative_pattern,
    s_pattern,
    s_table,
    always_true,
    always_false
};

struct label{
    label_kind k;
    const rbg_parser::game_move* a;
    uint structure_index;
};

class edge{
        uint local_register_endpoint_index;
        std::vector<label> label_list;
        int handle_labels(
            cpp_container& output,
            actions_compiler& ac,
            const std::vector<shift_table>& shift_tables,
            const std::vector<precomputed_pattern>& precomputed_patterns)const;
        void visit_node(cpp_container& output, uint current_state, actions_compiler& ac, const std::string& cell="current_cell", const std::string& kind_of_return="return;", bool revert_if_encountered=true)const;
        void visit_node_in_pattern(cpp_container& output, uint current_state, uint pattern_index, actions_compiler& ac, const std::string& cell="current_cell", const std::string& kind_of_return="return false;", bool revert_if_encountered=true)const;
    public:
        edge(uint local_register_endpoint_index, const std::vector<label>& label_list);
        void shift(uint shift_value);
        void inform_abut_state_deletion(uint deleted_index);
        uint get_endpoint(void)const;
        void print_transition_function(
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
        void print_transition_function_inside_pattern(
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
        int get_next_cell(uint current_cell, const rbg_parser::graph& board, const std::vector<precomputed_pattern>& pps)const;// -1 = invalid
        bool is_shift_table_with_multiple_choices(const std::vector<shift_table>& shift_tables)const;
};

#endif
