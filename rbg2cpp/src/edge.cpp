#include"edge.hpp"
#include"game_move.hpp"
#include"cpp_container.hpp"
#include"actions_compiler.hpp"
#include"declarations.hpp"
#include"state.hpp"
#include"compiler_options.hpp"
#include"graph.hpp"
#include"precomputed_pattern.hpp"
#include"shift_table.hpp"
#include"shift.hpp"

edge::edge(uint local_register_endpoint_index, const std::vector<label>& label_list):
local_register_endpoint_index(local_register_endpoint_index),
label_list(label_list){
}

void edge::shift(uint shift_value){
    local_register_endpoint_index += shift_value;
}

void edge::inform_abut_state_deletion(uint deleted_index){
    if(local_register_endpoint_index>deleted_index)
        --local_register_endpoint_index;
}

uint edge::get_endpoint(void)const{
    return local_register_endpoint_index;
}

int edge::handle_labels(
    cpp_container& output,
    actions_compiler& ac,
    const std::vector<shift_table>& shift_tables,
    const std::vector<precomputed_pattern>& precomputed_patterns)const{
    for(uint i=0;i<label_list.size();++i){
        const auto& el = label_list[i];
        switch(el.k){
            case action:
                el.a->accept(ac);
                break;
            case positive_pattern:
                ac.finallize();
                output.add_source_line("if(not evaluate"+std::to_string(el.structure_index)+"(current_cell)){");
                ac.insert_reverting_sequence(output);
                output.add_source_line("}");
                break;
            case negative_pattern:
                ac.finallize();
                output.add_source_line("if(evaluate"+std::to_string(el.structure_index)+"(current_cell)){");
                ac.insert_reverting_sequence(output);
                output.add_source_line("}");
                break;
            case s_pattern:
                precomputed_patterns[el.structure_index].print_inside_transition(output, ac);
                break;
            case s_table:
                if(shift_tables[el.structure_index].can_be_backtraced()){
                    assert(i==label_list.size()-1);
                    return el.structure_index;
                }
                else{
                    output.add_source_line("current_cell = shift_table"+std::to_string(el.structure_index)+"[current_cell];");
                    ac.notify_about_cell_change();
                }
                break;
            case always_true:
                break;
            case always_false:
                ac.insert_reverting_sequence(output);
                break;
        }
    }
    return -1;
}

void edge::visit_node(cpp_container& output, uint current_state, actions_compiler& ac, const std::string& cell, const std::string& kind_of_return, bool revert_if_encountered)const{
    ac.finallize();
    output.add_source_line("if(cache.is_set("+std::to_string(current_state)+", "+cell+"-1)){");
    if(revert_if_encountered)
        ac.insert_unended_reverting_sequence(output);
    output.add_source_line(kind_of_return);
    output.add_source_line("}");
    output.add_source_line("cache.set("+std::to_string(current_state)+", "+cell+"-1);");
}

void edge::visit_node_in_pattern(cpp_container& output, uint current_state, uint pattern_index, actions_compiler& ac, const std::string& cell, const std::string& kind_of_return, bool revert_if_encountered)const{
    ac.finallize();
    output.add_source_line("if(cache.pattern_is_set"+std::to_string(pattern_index)+"("+std::to_string(current_state)+", "+cell+"-1)){");
    if(revert_if_encountered)
        ac.insert_unended_reverting_sequence(output);
    output.add_source_line(kind_of_return);
    output.add_source_line("}");
    output.add_source_line("cache.pattern_set"+std::to_string(pattern_index)+"("+std::to_string(current_state)+", "+cell+"-1);");
}

void edge::print_transition_function(
    uint from_state,
    cpp_container& output,
    const std::map<rbg_parser::token, uint>& pieces_to_id,
    const std::map<rbg_parser::token, uint>& edges_to_id,
    const std::map<rbg_parser::token, uint>& variables_to_id,
    const rbg_parser::declarations& decl,
    const std::vector<state>& local_register,
    const std::vector<shift_table>& shift_tables,
    const std::vector<precomputed_pattern>& precomputed_patterns,
    bool stop_after_first)const{
    if(stop_after_first){
        output.add_header_line("bool get_any_move_"+std::to_string(from_state)+"_"+std::to_string(local_register_endpoint_index)+"(int current_cell);");
        output.add_source_line("bool next_states_iterator::get_any_move_"+std::to_string(from_state)+"_"+std::to_string(local_register_endpoint_index)+"(int current_cell){");
    }
    else{
        output.add_header_line("void get_all_moves_"+std::to_string(from_state)+"_"+std::to_string(local_register_endpoint_index)+"(int current_cell);");
        output.add_source_line("void next_states_iterator::get_all_moves_"+std::to_string(from_state)+"_"+std::to_string(local_register_endpoint_index)+"(int current_cell){");
    }
    std::string final_action = (stop_after_first?"return false;":"return;");
    std::string pusher = "push()";
    std::string level_getter = "get_level()";
    std::string level_reverter = "revert_to_level";
    std::string decision_stack = "decision_points";
    actions_compiler ac(output,pieces_to_id,edges_to_id,variables_to_id,decl,pusher,level_getter,level_reverter,final_action,false);
    int last_state_to_check = -1;
    int encountered_multi_shift_table = handle_labels(output,ac,shift_tables,precomputed_patterns);
    uint current_state = local_register_endpoint_index;
    while(not ac.is_ready_to_report() and local_register[current_state].is_no_choicer() and encountered_multi_shift_table == -1){
        if(local_register[current_state].can_be_checked_for_visit()){
            if(local_register[current_state].get_only_exit().label_list.empty())
                last_state_to_check = current_state;
            else{
                visit_node(output, current_state,ac,"current_cell",final_action);
                last_state_to_check = -1;
            }
        }
        else if(not local_register[current_state].get_only_exit().label_list.empty() and last_state_to_check >= 0){
            visit_node(output,last_state_to_check,ac,"current_cell",final_action);
            last_state_to_check = -1;
        }
        encountered_multi_shift_table = local_register[current_state].get_only_exit().handle_labels(output,ac,shift_tables,precomputed_patterns);
        current_state = local_register[current_state].get_only_exit().local_register_endpoint_index;
    }
    if(encountered_multi_shift_table >= 0){
        ac.notify_about_modifier();
        output.add_source_line("for(const auto el: shift_table"+std::to_string(encountered_multi_shift_table)+"[current_cell]){");
        visit_node(output, current_state, ac, "el","continue;",false);
        if(stop_after_first)
            local_register[current_state].print_recursive_calls_for_any_getter(current_state,output,ac,"el");
        else
            local_register[current_state].print_recursive_calls_for_all_getter(current_state,output,"el");
        output.add_source_line("}");
        ac.insert_reverting_sequence(output);
        output.add_source_line("}");
        output.add_source_line("");
        return;
    }
    if(local_register[current_state].can_be_checked_for_visit())
        last_state_to_check = current_state;
    if(last_state_to_check >= 0)
        visit_node(output, last_state_to_check,ac,"current_cell",final_action);
    ac.finallize();
    if(ac.is_ready_to_report()){
        output.add_source_line("moves.emplace_back(board_list,variables_list,"+std::to_string(ac.get_next_player())+",current_cell,"+std::to_string(current_state)+");");
        ac.insert_unended_reverting_sequence(output);
        if(stop_after_first)
            output.add_source_line("return true;");
        else
            output.add_source_line("return;");
    }
    else{
        if(stop_after_first)
            local_register[current_state].print_recursive_calls_for_any_getter(current_state,output,ac);
        else
            local_register[current_state].print_recursive_calls_for_all_getter(current_state,output);
        ac.insert_reverting_sequence(output);
    }
    output.add_source_line("}");
    output.add_source_line("");
}

void edge::print_transition_function_inside_pattern(
    uint from_state,
    uint pattern_index,
    cpp_container& output,
    const std::map<rbg_parser::token, uint>& pieces_to_id,
    const std::map<rbg_parser::token, uint>& edges_to_id,
    const std::map<rbg_parser::token, uint>& variables_to_id,
    const rbg_parser::declarations& decl,
    const std::vector<state>& local_register,
    const std::vector<shift_table>& shift_tables,
    const std::vector<precomputed_pattern>& precomputed_patterns)const{
    output.add_header_line("bool get_pattern_value"+std::to_string(pattern_index)+"_"+std::to_string(from_state)+"_"+std::to_string(local_register_endpoint_index)+"(int current_cell);");
    output.add_source_line("bool next_states_iterator::get_pattern_value"+std::to_string(pattern_index)+"_"+std::to_string(from_state)+"_"+std::to_string(local_register_endpoint_index)+"(int current_cell){");
    std::string pusher = "pattern_push"+std::to_string(pattern_index)+"()";
    std::string level_getter = "pattern_get_level"+std::to_string(pattern_index)+"()";
    std::string level_reverter = "pattern_revert_to_level"+std::to_string(pattern_index);
    std::string final_action = "return false;";
    actions_compiler ac(output,pieces_to_id,edges_to_id,variables_to_id,decl,pusher,level_getter,level_reverter,final_action,true);
    std::string decision_stack = "pattern_decision_points"+std::to_string(pattern_index);
    int last_state_to_check = -1;
    int encountered_multi_shift_table = handle_labels(output,ac,shift_tables,precomputed_patterns);
    uint current_state = local_register_endpoint_index;
    while(local_register[current_state].is_no_choicer() and encountered_multi_shift_table == -1){
        if(local_register[current_state].can_be_checked_for_visit()){
            if(local_register[current_state].get_only_exit().label_list.empty())
                last_state_to_check = current_state;
            else{
                visit_node_in_pattern(output,current_state,pattern_index,ac);
                last_state_to_check = -1;
            }
        }
        else if(not local_register[current_state].get_only_exit().label_list.empty() and last_state_to_check >= 0){
            visit_node_in_pattern(output,last_state_to_check,pattern_index,ac);
            last_state_to_check = -1;
        }
        encountered_multi_shift_table = local_register[current_state].get_only_exit().handle_labels(output,ac,shift_tables,precomputed_patterns);
        current_state = local_register[current_state].get_only_exit().local_register_endpoint_index;
    }
    if(encountered_multi_shift_table >= 0){
        ac.notify_about_modifier();
        output.add_source_line("for(const auto el: shift_table"+std::to_string(encountered_multi_shift_table)+"[current_cell]){");
        visit_node_in_pattern(output, current_state, pattern_index, ac, "el","continue;",false);
        local_register[current_state].print_recursive_calls_for_pattern(current_state,output,ac,pattern_index,"el");
        output.add_source_line("}");
        ac.insert_reverting_sequence(output);
        output.add_source_line("}");
        output.add_source_line("");
        return;
    }
    if(local_register[current_state].can_be_checked_for_visit())
        last_state_to_check = current_state;
    if(last_state_to_check >= 0)
        visit_node_in_pattern(output,current_state,pattern_index,ac);
    ac.finallize();
    assert(not ac.is_ready_to_report());
    if(local_register[current_state].is_dead_end()){
        ac.insert_unended_reverting_sequence(output);
        output.add_source_line("return true;");
    }
    else{
        local_register[current_state].print_recursive_calls_for_pattern(current_state, output, ac, pattern_index);
        ac.insert_reverting_sequence(output);
    }
    output.add_source_line("}");
    output.add_source_line("");
}

int edge::get_next_cell(uint current_cell, const rbg_parser::graph& board, const std::vector<precomputed_pattern>& pps)const{
    for(const auto& el: label_list)
        switch(el.k){
            case action:
                {
                    const auto& outgoing_edges = board.get_outgoing_edges(current_cell);
                    const rbg_parser::shift* s = dynamic_cast<const rbg_parser::shift*>(el.a);
                    auto it = outgoing_edges.find(s->get_content());
                    if(it == outgoing_edges.end())
                        return -1;
                    current_cell = it->second;
                }
                break;
            case positive_pattern:
            case negative_pattern:
                assert(false);
                break;
            case s_pattern:
                if(not pps[el.structure_index].evaluates_to_true(current_cell))
                    return -1;
                break;
            case s_table:
                assert(false); // no recursive shift tables
            case always_true:
                break;
            case always_false:
                return -1;
                break;
        }
    return current_cell;
}

bool edge::is_shift_table_with_multiple_choices(const std::vector<shift_table>& shift_tables)const{
    return label_list.size() == 1 and label_list[0].k == s_table and shift_tables[label_list[0].structure_index].can_be_backtraced();
}
