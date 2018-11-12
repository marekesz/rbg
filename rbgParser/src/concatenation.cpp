#include"concatenation.hpp"
#include"printer_helpers.hpp"

namespace rbg_parser{

concatenation::concatenation(std::vector<std::unique_ptr<game_move>>&& content):
content(std::move(content)){
}

bool concatenation::modifies(void)const{
    for(const auto& el: content)
        if(el->modifies())
            return true;
    return false;
}

std::unique_ptr<game_move> concatenation::simplify(void){
    std::vector<std::unique_ptr<game_move>> result;
    for(uint i=0;i<content.size();++i)
        result.push_back(content[i]->simplify());
    if(result.size()==1)
        return std::move(result[0]);
    else
        return std::unique_ptr<game_move>(new concatenation(std::move(result)));
}

void concatenation::accept(abstract_dispatcher& dispatcher)const{
    dispatcher.dispatch(*this);
}

std::string concatenation::to_rbg(uint indent)const{
    std::string result = "";
    for(uint i=0;i<content.size();++i){
        result += open_bracket_if_necessary(priority(),content[i]->priority());
        result += content[i]->to_rbg(indent);
        result += close_bracket_if_necessary(priority(),content[i]->priority());
    }
    return result;
}

std::string concatenation::to_rbg()const{
    std::string result = "";
    for(uint i=0;i<content.size();++i){
        result += open_bracket_if_necessary(priority(),content[i]->priority());
        result += content[i]->to_rbg();
        result += close_bracket_if_necessary(priority(),content[i]->priority());
    }
    return result;
}

std::unique_ptr<game_move> concatenation::flatten(void){
    std::vector<std::unique_ptr<game_move>> result;
    for(auto& el: content)
        el->gather_concatenation_elements(result);
    return std::unique_ptr<game_move>(new concatenation(std::move(result)));
}

void concatenation::gather_concatenation_elements(std::vector<std::unique_ptr<game_move>>& elements){
    for(auto& el: content)
        el->gather_concatenation_elements(elements);
}

straightness_result concatenation::compute_k_straightness(void)const{
    auto current_max = standard_non_switch();
    for(const auto& el: content)
        current_max.concatenate_result(el->compute_k_straightness());
    return current_max;
}

const std::vector<std::unique_ptr<game_move>>& concatenation::get_content(void)const{
    return content;
}

void concatenation::add_move(std::unique_ptr<game_move> m){
    content.push_back(std::move(m));
}

std::unique_ptr<game_move> concatenation::copy(void)const{
    std::vector<std::unique_ptr<game_move>> result_content;
    for(const auto& el: content)
        result_content.push_back(el->copy());
    return std::unique_ptr<game_move>(new concatenation(std::move(result_content)));
}

}
