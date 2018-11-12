#include"arithmetic_operation.hpp"
#include"abstract_dispatcher.hpp"
#include"printer_helpers.hpp"
#include<cassert>

namespace rbg_parser{

arithmetic_operation::arithmetic_operation(std::vector<std::unique_ptr<arithmetic_expression>>&& content, operation kind_of_operation):
content(std::move(content)),
kind_of_operation(kind_of_operation){
}

void arithmetic_operation::accept(abstract_dispatcher& dispatcher)const{
    dispatcher.dispatch(*this);
}

std::unique_ptr<arithmetic_expression> arithmetic_operation::simplify(void){
    std::vector<std::unique_ptr<arithmetic_expression>> result;
    for(uint i=0;i<content.size();++i)
        result.push_back(content[i]->simplify());
    if(result.size()==1)
        return std::move(result[0]);
    else
        return std::unique_ptr<arithmetic_expression>(new arithmetic_operation(std::move(result), kind_of_operation));
}

std::unique_ptr<arithmetic_expression> arithmetic_operation::flatten(void){
    std::vector<std::unique_ptr<arithmetic_expression>> result;
    if(kind_of_operation == addition)
        for(auto& el: content)
            el->gather_sum_elements(result);
    else if(kind_of_operation == multiplication)
        for(auto& el: content)
            el->gather_multiply_elements(result);
    else
        for(auto& el: content)
            result.push_back(el->flatten());
    return std::unique_ptr<arithmetic_expression>(new arithmetic_operation(std::move(result), kind_of_operation));
}

void arithmetic_operation::gather_sum_elements(std::vector<std::unique_ptr<arithmetic_expression>>& elements){
    if(kind_of_operation == addition)
        for(auto& el: content)
            el->gather_sum_elements(elements);
    else
        elements.push_back(flatten());
}

void arithmetic_operation::gather_multiply_elements(std::vector<std::unique_ptr<arithmetic_expression>>& elements){
    if(kind_of_operation == multiplication)
        for(auto& el: content)
            el->gather_multiply_elements(elements);
    else
        elements.push_back(flatten());
}

uint arithmetic_operation::priority(void)const{
    switch(kind_of_operation){
        case addition:
        case subtraction:
            return 1;
        case multiplication:
        case division:
            return 3;
        default:
            assert(false);
    }
}

std::string arithmetic_operation::to_rbg(uint)const{
    return to_rbg();
}

std::string arithmetic_operation::to_rbg()const{
    std::string result = "";
    std::string op_string;
    switch(kind_of_operation){
        case addition:
            op_string = "+";
            break;
        case subtraction:
            op_string = "-";
            break;
        case multiplication:
            op_string = "*";
            break;
        case division:
            op_string = "/";
            break;
        default:
            assert(false);
    }
    for(uint i=0;i<content.size();++i){
        result += (i==0 ? "" : op_string);
        if(kind_of_operation == subtraction or kind_of_operation == division){
            result += (priority() >= content[i]->priority() ? "(" : "");
            result += content[i]->to_rbg();
            result += (priority() >= content[i]->priority() ? ")" : "");
        }
        else{
            result += open_bracket_if_necessary(priority(),content[i]->priority());
            result += content[i]->to_rbg();
            result += close_bracket_if_necessary(priority(),content[i]->priority());
        }
    }
    return result;
}

const std::vector<std::unique_ptr<arithmetic_expression>>& arithmetic_operation::get_content(void)const{
    return content;
}

operation arithmetic_operation::get_operation(void)const{
    return kind_of_operation;
}

std::unique_ptr<arithmetic_expression> arithmetic_operation::copy(void)const{
    std::vector<std::unique_ptr<arithmetic_expression>> result_content;
    for(const auto& el: content)
        result_content.push_back(el->copy());
    return std::unique_ptr<arithmetic_expression>(new arithmetic_operation(std::move(result_content), kind_of_operation));
}

}
