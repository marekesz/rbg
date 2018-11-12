#include"sum.hpp"
#include"printer_helpers.hpp"

namespace rbg_parser{

sum::sum(std::vector<std::unique_ptr<game_move>>&& content):
content(std::move(content)){
}

bool sum::modifies(void)const{
    for(const auto& el: content)
        if(el->modifies())
            return true;
    return false;
}

std::unique_ptr<game_move> sum::simplify(void){
    std::vector<std::unique_ptr<game_move>> result;
    for(uint i=0;i<content.size();++i)
        result.push_back(content[i]->simplify());
    if(result.size()==1)
        return std::move(result[0]);
    else
        return std::unique_ptr<game_move>(new sum(std::move(result)));
}

void sum::accept(abstract_dispatcher& dispatcher)const{
    dispatcher.dispatch(*this);
}

std::string sum::to_rbg(uint indent)const{
    std::string result = "";
    for(uint i=0;i<content.size();++i){
        result += (i==0 ? "\n":"\n+ ");
        result += open_bracket_if_necessary(priority(),content[i]->priority());
        result += content[i]->to_rbg(indent+1);
        result += close_bracket_if_necessary(priority(),content[i]->priority());
    }
    result += "\n";//+print_tabs(indent);
    return result;
}

std::string sum::to_rbg()const{
    std::string result = "";
    for(uint i=0;i<content.size();++i){
        result += (i==0 ? "" : " + ");
        result += open_bracket_if_necessary(priority(),content[i]->priority());
        result += content[i]->to_rbg();
        result += close_bracket_if_necessary(priority(),content[i]->priority());
    }
    return result;
}

std::unique_ptr<game_move> sum::flatten(void){
    std::vector<std::unique_ptr<game_move>> result;
    for(auto& el: content)
        el->gather_sum_elements(result);
    return std::unique_ptr<game_move>(new sum(std::move(result)));
}

void sum::gather_sum_elements(std::vector<std::unique_ptr<game_move>>& elements){
    for(auto& el: content)
        el->gather_sum_elements(elements);
}

straightness_result sum::compute_k_straightness(void)const{
    auto current_max = empty_move();
    for(const auto& el: content)
        current_max.max_of_results(el->compute_k_straightness());
    return current_max;
}

const std::vector<std::unique_ptr<game_move>>& sum::get_content(void)const{
    return content;
}

void sum::add_move(std::unique_ptr<game_move> m){
    content.push_back(std::move(m));
}

std::unique_ptr<game_move> sum::copy(void)const{
    std::vector<std::unique_ptr<game_move>> result_content;
    for(const auto& el: content)
        result_content.push_back(el->copy());
    return std::unique_ptr<game_move>(new sum(std::move(result_content)));
}

}
