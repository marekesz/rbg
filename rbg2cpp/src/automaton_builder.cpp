#include"automaton_builder.hpp"
#include"game_move.hpp"
#include"state.hpp"
#include"edge.hpp"
#include"arithmetic_comparison.hpp"
#include"move_check.hpp"
#include"shift.hpp"
#include"ons.hpp"
#include"offs.hpp"
#include"assignments.hpp"
#include"switch.hpp"
#include"star_move.hpp"
#include"concatenation.hpp"
#include"sum.hpp"
#include"compiler_options.hpp"
#include"graph.hpp"

constexpr uint PATH_SIZE_FOR_SHIFT_TABLE = 2;

automaton_builder::automaton_builder(
    const rbg_parser::graph& board,
    std::vector<automaton>& pattern_automata,
    std::vector<shift_table>& shift_tables,
    std::vector<precomputed_pattern>& precomputed_patterns,
    std::vector<label>& current_block,
    std::vector<label>& current_shift_block,
    const compiler_options& opts):
local_copy_automaton(),
local_copy_shift_automaton(),
currently_modified_automaton(local_copy_automaton),
currently_modified_shift_automaton(local_copy_shift_automaton),
has_automaton(false),
has_shift_automaton(false),
board(board),
pattern_automata(pattern_automata),
shift_tables(shift_tables),
precomputed_patterns(precomputed_patterns),
current_block(current_block),
current_shift_block(current_shift_block),
opts(opts){
}

automaton_builder::automaton_builder(
    const rbg_parser::graph& board,
    std::vector<automaton>& pattern_automata,
    std::vector<shift_table>& shift_tables,
    std::vector<precomputed_pattern>& precomputed_patterns,
    std::vector<label>& current_block,
    std::vector<label>& current_shift_block,
    automaton& upper_level_automaton,
    automaton& upper_level_shift_automaton,
    const compiler_options& opts):
local_copy_automaton(),
local_copy_shift_automaton(),
currently_modified_automaton(upper_level_automaton),
currently_modified_shift_automaton(upper_level_shift_automaton),
has_automaton(false),
has_shift_automaton(false),
board(board),
pattern_automata(pattern_automata),
shift_tables(shift_tables),
precomputed_patterns(precomputed_patterns),
current_block(current_block),
current_shift_block(current_shift_block),
opts(opts){
}

automaton_builder automaton_builder::delegate_builder(){
    automaton_builder b(
        board,
        pattern_automata,
        shift_tables,
        precomputed_patterns,
        current_block,
        current_shift_block,
        currently_modified_automaton,
        currently_modified_shift_automaton,
        opts);
    b.has_automaton = has_automaton;
    b.has_shift_automaton = has_shift_automaton;
    return b;
}

bool automaton_builder::only_shifts(void)const{
    return not has_automaton and current_block.empty();
}

void automaton_builder::dispatch(const rbg_parser::arithmetic_comparison& m){
    end_shift_automaton();
    current_block.push_back({action,&m,0});
}

void automaton_builder::dispatch(const rbg_parser::move_check& m){
    std::vector<label> block;
    std::vector<label> shift_block;
    automaton_builder element_builder(
        board,
        pattern_automata,
        shift_tables,
        precomputed_patterns,
        block,
        shift_block,
        opts);
    m.get_content()->accept(element_builder);
    if(element_builder.only_shifts()){
        auto table = element_builder.gather_shift_so_far_into_table();
        precomputed_patterns.push_back(table.transform_into_pattern());
        if(m.is_negated())
            precomputed_patterns.back().negate();
        current_shift_block.push_back({s_pattern,nullptr,uint(precomputed_patterns.size()-1)});
    }
    else{
        end_shift_automaton();
        pattern_automata.push_back(element_builder.get_final_result());
        current_block.push_back({(m.is_negated()?negative_pattern:positive_pattern),nullptr,uint(pattern_automata.size()-1)});
    }
}

void automaton_builder::dispatch(const rbg_parser::shift& m){
    if(opts.enabled_shift_tables())
        current_shift_block.push_back({action,&m,0});
    else
        current_block.push_back({action,&m,0});
}

void automaton_builder::dispatch(const rbg_parser::ons& m){
    end_shift_automaton();
    current_block.push_back({action,&m,0});
}

void automaton_builder::dispatch(const rbg_parser::off& m){
    end_shift_automaton();
    current_block.push_back({action,&m,0});
}

void automaton_builder::dispatch(const rbg_parser::assignment& m){
    end_shift_automaton();
    current_block.push_back({action,&m,0});
}

void automaton_builder::build_automaton_from_actions_so_far(void){
    if(not current_block.empty())
        concat_automaton_to_result_so_far(edge_automaton(current_block));
    current_block.clear();
}

void automaton_builder::concat_automaton_to_result_so_far(automaton&& a){
    if(has_automaton)
        currently_modified_automaton.concat_automaton(std::move(a));
    else
        currently_modified_automaton = std::move(a);
    has_automaton = true;
}

void automaton_builder::concat_shift_automaton_to_result_so_far(automaton&& a){
    if(has_shift_automaton)
        currently_modified_shift_automaton.concat_automaton(std::move(a));
    else
        currently_modified_shift_automaton = std::move(a);
    has_shift_automaton = true;
}

bool automaton_builder::worth_creating_shift_table(void)const{
    return has_shift_automaton or current_shift_block.size() >= PATH_SIZE_FOR_SHIFT_TABLE;
}

shift_table automaton_builder::gather_shift_so_far_into_table(void){
    build_shift_automaton_from_actions_so_far();
    auto result = currently_modified_shift_automaton.generate_shift_table(board, precomputed_patterns);
    has_shift_automaton = false;
    return result;
}

void automaton_builder::end_shift_automaton(void){
    if(worth_creating_shift_table()){
        auto shift_table_index = insert_shift_table(shift_tables, gather_shift_so_far_into_table());
        if(shift_tables[shift_table_index].can_be_backtraced()){
            build_automaton_from_actions_so_far();
            std::vector<label> shift_label;
            shift_label.push_back({s_table,nullptr,shift_table_index});
            auto last_edge = edge_automaton(shift_label);
            last_edge.mark_end_as_outgoing_usable();
            concat_automaton_to_result_so_far(std::move(last_edge));
        }
        else
            current_block.push_back({s_table,nullptr,shift_table_index});
    }
    else{
        if(has_shift_automaton){
            build_automaton_from_actions_so_far();
            concat_automaton_to_result_so_far(std::move(currently_modified_shift_automaton));
            has_shift_automaton = false;
        }
        current_block.insert(current_block.end(), current_shift_block.begin(), current_shift_block.end());
        current_shift_block.clear();
    }
}

void automaton_builder::handle_any_switch(const rbg_parser::game_move& m){
    end_shift_automaton();
    current_block.push_back({action,&m,0});
    build_automaton_from_actions_so_far();
    currently_modified_automaton.mark_end_as_outgoing_usable();
}

void automaton_builder::dispatch(const rbg_parser::player_switch& m){
    handle_any_switch(m);
}

void automaton_builder::dispatch(const rbg_parser::keeper_switch& m){
    handle_any_switch(m);
}

void automaton_builder::build_shift_automaton_from_actions_so_far(void){
    if(not current_shift_block.empty())
        concat_shift_automaton_to_result_so_far(edge_automaton(current_shift_block));
    current_shift_block.clear();
}

automaton automaton_builder::get_only_shifts_final_result(void){
    assert(only_shifts());
    build_shift_automaton_from_actions_so_far();
    return std::move(currently_modified_shift_automaton);
}

void automaton_builder::dispatch(const rbg_parser::star_move& m){
    std::vector<label> block;
    std::vector<label> shift_block;
    automaton_builder element_builder(
        board,
        pattern_automata,
        shift_tables,
        precomputed_patterns,
        block,
        shift_block,
        opts);
    m.get_content()->accept(element_builder);
    if(element_builder.only_shifts()){
        auto temp_automaton = element_builder.get_only_shifts_final_result();
        temp_automaton.starify_automaton();
        build_shift_automaton_from_actions_so_far();
        concat_shift_automaton_to_result_so_far(std::move(temp_automaton));
    }
    else{
        end_shift_automaton();
        auto temp_automaton = element_builder.get_final_result();
        temp_automaton.starify_automaton();
        build_automaton_from_actions_so_far();
        concat_automaton_to_result_so_far(std::move(temp_automaton));
    }
    assert(block.empty() and shift_block.empty());
}

void automaton_builder::dispatch(const rbg_parser::sum& m){
    std::vector<automaton> elements;
    std::vector<automaton> only_shift_elements;
    bool encountered_worth_shift_automaton = false;
    for(const auto& el: m.get_content()){
        std::vector<label> block;
        std::vector<label> shift_block;
        automaton_builder element_builder(
            board,
            pattern_automata,
            shift_tables,
            precomputed_patterns,
            block,
            shift_block,
            opts);
        el->accept(element_builder);
        if(element_builder.only_shifts()){
            if(element_builder.worth_creating_shift_table() or only_shift_elements.size() >= 1)
                encountered_worth_shift_automaton = true;
            only_shift_elements.push_back(element_builder.get_only_shifts_final_result());
        }
        else
            elements.push_back(element_builder.get_final_result());
        assert(block.empty() and shift_block.empty());
    }
    if(only_shift_elements.empty()){
        end_shift_automaton();
        build_automaton_from_actions_so_far();
        concat_automaton_to_result_so_far(sum_of_automatons(std::move(elements)));
    }
    else if(elements.empty()){
        build_shift_automaton_from_actions_so_far();
        concat_shift_automaton_to_result_so_far(sum_of_automatons(std::move(only_shift_elements)));
    }
    else{
        end_shift_automaton();
        if(encountered_worth_shift_automaton){
            auto shift_automaton = sum_of_automatons(std::move(only_shift_elements));
            auto shift_table_index = insert_shift_table(shift_tables, shift_automaton.generate_shift_table(board,precomputed_patterns));
            std::vector<label> last_edge;
            last_edge.push_back({s_table,nullptr,shift_table_index});
            auto e = edge_automaton(last_edge);
            e.mark_end_as_outgoing_usable();
            elements.push_back(std::move(e));
        }
        else{
            for(auto& el: only_shift_elements)
                elements.push_back(std::move(el));
        }
        build_automaton_from_actions_so_far();
        concat_automaton_to_result_so_far(sum_of_automatons(std::move(elements)));
    }
}

void automaton_builder::dispatch(const rbg_parser::concatenation& m){
    auto b(delegate_builder());
    for(const auto& el: m.get_content())
        el->accept(b);
    has_automaton = b.has_automaton;
    has_shift_automaton = b.has_shift_automaton;
}

automaton automaton_builder::get_final_result(void){
    end_shift_automaton();
    if(has_automaton){
        if(not current_block.empty())
            currently_modified_automaton.concat_automaton(edge_automaton(current_block));
        current_block.clear();
        return std::move(currently_modified_automaton);
    }
    else{
        auto result = edge_automaton(current_block);
        current_block.clear();
        return result;
    }
}
