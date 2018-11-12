#include"actions_compiler.hpp"
#include"declarations.hpp"
#include"cpp_container.hpp"
#include"arithmetics_printer.hpp"
#include"game_move.hpp"
#include"shift.hpp"
#include"ons.hpp"
#include"offs.hpp"
#include"assignments.hpp"
#include"switch.hpp"
#include"arithmetic_comparison.hpp"

actions_compiler::actions_compiler(
    cpp_container& output,
    const std::map<rbg_parser::token, uint>& pieces_to_id,
    const std::map<rbg_parser::token, uint>& edges_to_id,
    const std::map<rbg_parser::token, uint>& variables_to_id,
    const rbg_parser::declarations& decl,
    const std::string& cache_pusher,
    const std::string& cache_level_getter,
    const std::string& cache_level_reverter,
    const std::string& final_action,
    bool inside_pattern):
    output(output),
    pieces_to_id(pieces_to_id),
    edges_to_id(edges_to_id),
    variables_to_id(variables_to_id),
    decl(decl),
    cache_pusher(cache_pusher),
    cache_level_getter(cache_level_getter),
    cache_level_reverter(cache_level_reverter),
    final_action(final_action),
    reverting_stack(),
    encountered_board_change(false),
    encountered_variable_change(false),
    should_check_cell_correctness(false),
    has_modifier(false),
    has_saved_cache_level(false),
    next_player(-1),
    inside_pattern(inside_pattern){
}

void actions_compiler::dispatch(const rbg_parser::shift& m){
    output.add_source_line("current_cell = cell_neighbors[current_cell]["+std::to_string(edges_to_id.at(m.get_content()))+"];");
    should_check_cell_correctness = true;
}

void actions_compiler::push_changes_on_board_list(cpp_container& output, const std::string& piece_id){
    if(not inside_pattern){
        if(not encountered_board_change)
            output.add_source_line("const auto previous_board_changes_list = board_list.size();");
        output.add_source_line("board_list.emplace_back(current_cell,"+piece_id+");");
        encountered_board_change = true;
    }
}

void actions_compiler::save_board_change_for_later_revert(cpp_container& output, uint piece_id){
    output.add_source_line("int board_change"+std::to_string(reverting_stack.size())+"_cell = current_cell;");
    output.add_source_line("int board_change"+std::to_string(reverting_stack.size())+"_piece = state_to_change.pieces[current_cell];");
    reverting_stack.push_back({board_change,piece_id});
}

void actions_compiler::revert_board_change(cpp_container& output, uint piece_id, uint stack_position)const{
    output.add_source_line("--state_to_change.pieces_count["+std::to_string(piece_id)+"];");
    output.add_source_line("++state_to_change.pieces_count[board_change"+std::to_string(stack_position)+"_piece];");
    output.add_source_line("state_to_change.pieces[board_change"+std::to_string(stack_position)+"_cell] = board_change"+std::to_string(stack_position)+"_piece;");
}

void actions_compiler::dispatch(const rbg_parser::off& m){
    check_cell_correctness();
    save_board_change_for_later_revert(output,pieces_to_id.at(m.get_piece()));
    push_changes_on_board_list(output, std::to_string(pieces_to_id.at(m.get_piece())));
    output.add_source_line("--state_to_change.pieces_count[state_to_change.pieces[current_cell]];");
    output.add_source_line("++state_to_change.pieces_count["+std::to_string(pieces_to_id.at(m.get_piece()))+"];");
    output.add_source_line("state_to_change.pieces[current_cell] = "+std::to_string(pieces_to_id.at(m.get_piece()))+";");
    has_modifier = true;
}

void actions_compiler::dispatch(const rbg_parser::ons& m){
    if(m.get_legal_ons().size() == 0)
        insert_reverting_sequence(output);
    else if(m.get_legal_ons().size() < pieces_to_id.size()){
        check_cell_correctness();
        output.add_source_line("switch(state_to_change.pieces[current_cell]){");
        if(m.get_legal_ons().size() < pieces_to_id.size()/2+1){
            for(const auto& el: m.get_legal_ons())
                output.add_source_line("case "+std::to_string(pieces_to_id.at(el))+":");
            output.add_source_line("break;");
            output.add_source_line("default:");
            insert_reverting_sequence(output);
        }
        else{
            for(const auto& el: pieces_to_id)
                if(not m.get_legal_ons().count(el.first))
                    output.add_source_line("case "+std::to_string(el.second)+":");
            insert_reverting_sequence(output);
            output.add_source_line("default:");
            output.add_source_line("break;");
        }
        output.add_source_line("}");
    }
}

void actions_compiler::push_changes_on_variables_list(cpp_container& output, const std::string& variable_id, const std::string& value){
    if(not inside_pattern){
        if(not encountered_variable_change)
            output.add_source_line("const auto previous_variables_changes_list = variables_list.size();");
        output.add_source_line("variables_list.emplace_back("+variable_id+","+value+");");
        encountered_variable_change = true;
    }
}

void actions_compiler::save_variable_change_for_later_revert(cpp_container& output, uint variable_id){
    output.add_source_line("int variable_change"+std::to_string(reverting_stack.size())+" = state_to_change.variables["+std::to_string(variable_id)+"];");
    reverting_stack.push_back({variable_change,variable_id});
}

void actions_compiler::revert_variable_change(cpp_container& output, uint variable_id, uint stack_position)const{
    output.add_source_line("state_to_change.variables["+std::to_string(variable_id)+"] = variable_change"+std::to_string(stack_position)+";");
}

void actions_compiler::dispatch(const rbg_parser::assignment& m){
    const auto& left_side = m.get_left_side();
    uint bound = 0;
    if(decl.get_legal_variables().count(left_side))
        bound = decl.get_variable_bound(left_side);
    else
        bound = decl.get_player_bound(left_side);
    arithmetics_printer right_side_printer(pieces_to_id, variables_to_id);
    m.get_right_side()->accept(right_side_printer);
    if(right_side_printer.can_be_precomputed()){
        if(right_side_printer.precomputed_value() < 0 or right_side_printer.precomputed_value() > int(bound))
            insert_reverting_sequence(output);
        else{
            save_variable_change_for_later_revert(output, variables_to_id.at(left_side));
            push_changes_on_variables_list(output, std::to_string(variables_to_id.at(left_side)), std::to_string(right_side_printer.precomputed_value()));
            output.add_source_line("state_to_change.variables["+std::to_string(variables_to_id.at(left_side))+"] = "+std::to_string(right_side_printer.precomputed_value())+";");
        }
    }
    else{
        std::string final_result = right_side_printer.get_final_result();
        output.add_source_line("if("+final_result+" > bounds["+std::to_string(variables_to_id.at(left_side))+"] or "+final_result+" <0){");
        insert_reverting_sequence(output);
        output.add_source_line("}");
        save_variable_change_for_later_revert(output, variables_to_id.at(left_side));
        push_changes_on_variables_list(output, std::to_string(variables_to_id.at(left_side)), final_result);
        output.add_source_line("state_to_change.variables["+std::to_string(variables_to_id.at(left_side))+"] = "+final_result+";");
    }
    has_modifier = true;
}

void actions_compiler::dispatch(const rbg_parser::player_switch& m){
    next_player = variables_to_id.at(m.get_player())+1;
}

void actions_compiler::dispatch(const rbg_parser::keeper_switch&){
    next_player = 0;
}

void actions_compiler::dispatch(const rbg_parser::arithmetic_comparison& m){
    arithmetics_printer left_side_printer(pieces_to_id, variables_to_id);
    arithmetics_printer right_side_printer(pieces_to_id, variables_to_id);
    m.get_left_side()->accept(left_side_printer);
    m.get_right_side()->accept(right_side_printer);
    if(left_side_printer.can_be_precomputed() and right_side_printer.can_be_precomputed()){
        bool can_pass_through = false;
        switch(m.get_kind_of_comparison()){
            case rbg_parser::eq:
                can_pass_through = (left_side_printer.precomputed_value() == right_side_printer.precomputed_value());
                break;
            case rbg_parser::neq:
                can_pass_through = (left_side_printer.precomputed_value() != right_side_printer.precomputed_value());
                break;
            case rbg_parser::le:
                can_pass_through = (left_side_printer.precomputed_value() < right_side_printer.precomputed_value());
                break;
            case rbg_parser::leq:
                can_pass_through = (left_side_printer.precomputed_value() <= right_side_printer.precomputed_value());
                break;
            case rbg_parser::ge:
                can_pass_through = (left_side_printer.precomputed_value() > right_side_printer.precomputed_value());
                break;
            case rbg_parser::geq:
                can_pass_through = (left_side_printer.precomputed_value() >= right_side_printer.precomputed_value());
                break;
            default:
                break;
        }
        if(not can_pass_through)
            insert_reverting_sequence(output);
    }
    else{
        std::string operation_character = "";
        switch(m.get_kind_of_comparison()){
            case rbg_parser::eq:
                operation_character = "!=";
                break;
            case rbg_parser::neq:
                operation_character = "==";
                break;
            case rbg_parser::le:
                operation_character = ">=";
                break;
            case rbg_parser::leq:
                operation_character = ">";
                break;
            case rbg_parser::ge:
                operation_character = "<=";
                break;
            case rbg_parser::geq:
                operation_character = "<";
                break;
            default:
                break;
        }
        output.add_source_line("if("+left_side_printer.get_final_result()+" "+operation_character+" "+right_side_printer.get_final_result()+"){");
        insert_reverting_sequence(output);
        output.add_source_line("}");
    }
}

void actions_compiler::insert_unended_reverting_sequence(cpp_container& output)const{
    if(has_saved_cache_level)
        output.add_source_line("cache."+cache_level_reverter+"(previous_cache_level);");
    if(not inside_pattern){
        if(encountered_board_change)
            output.add_source_line("board_list.resize(previous_board_changes_list);");
        if(encountered_variable_change)
            output.add_source_line("variables_list.resize(previous_variables_changes_list);");
    }
    for(uint i=reverting_stack.size();i>0;--i)
        switch(reverting_stack[i-1].type){
            case board_change:
                revert_board_change(output, reverting_stack[i-1].additional_info, i-1);
                break;
            case variable_change:
                revert_variable_change(output, reverting_stack[i-1].additional_info, i-1);
                break;
        }
}

void actions_compiler::insert_reverting_sequence(cpp_container& output)const{
    insert_unended_reverting_sequence(output);
    output.add_source_line(final_action);
}

void actions_compiler::finallize(void){
    check_cell_correctness();
    notify_about_modifier();
}

void actions_compiler::check_cell_correctness(void){
    if(should_check_cell_correctness){
        output.add_source_line("if(current_cell == 0){");
        insert_reverting_sequence(output);
        output.add_source_line("}");
    }
    should_check_cell_correctness = false;
}

void actions_compiler::notify_about_modifier(void){
    if(has_modifier){
        if(not has_saved_cache_level)
            output.add_source_line("unsigned int previous_cache_level = cache."+cache_level_getter+";");
        has_saved_cache_level = true;
        output.add_source_line("cache."+cache_pusher+";");
    }
    has_modifier = false;
}

bool actions_compiler::is_ready_to_report(void)const{
    return next_player >= 0;
}

void actions_compiler::notify_about_cell_change(void){
    should_check_cell_correctness = true;
}

int actions_compiler::get_next_player(void)const{
    assert(is_ready_to_report());
    return next_player;
}
