#include<cassert>

#include"macro_bank.hpp"

namespace rbg_parser{

macro::macro(std::vector<token>&& args, const std::vector<token>* d, uint b, uint e, uint co)noexcept:
arguments(std::move(args)),
data(d),
begin(b),
end(e),
context_order(co){
}

macro::macro(const macro& src)noexcept:
arguments(src.arguments),
data(src.data),
begin(src.begin),
end(src.end),
context_order(src.context_order){
}

macro::macro(macro&& src)noexcept:
arguments(src.arguments),
data(src.data),
begin(src.begin),
end(src.end),
context_order(src.context_order){
}

macro::~macro(void)noexcept{
}

slice macro::turn_into_slice(const std::vector<slice>& passed_values)const{
    assert(passed_values.size() == arguments.size());
    slice result(data,begin,end,context_order);
    for(uint i=0;i<passed_values.size();++i)
        result.add_replacement(arguments[i],passed_values[i]);
    return result;
}

uint macro::get_arity(void)const{
    return arguments.size();
}

uint macro::get_context_order(void)const{
    return context_order;
}

bool macro::is_empty(void)const{
    return end <= begin;
}

macro_bank::macro_bank(void)noexcept:
content(){
}

void macro_bank::add_macro(const token& name,macro&& m){
    assert(name.get_type() == identifier);
    if(!content.count(name))
        content.insert(std::make_pair(name,std::map<uint,macro>()));
    auto it = content.find(name);
    it->second.insert(std::make_pair(m.get_arity(),std::move(m)));
}

slice macro_bank::get_macro_slice(const token& name,const std::vector<slice>& passed_values)const{
    return content.find(name)->second.find(passed_values.size())->second.turn_into_slice(std::move(passed_values));
}

bool macro_bank::could_be_macro_name(const token& name)const{
    return content.find(name) != content.end();
}

bool macro_bank::is_zero_arity_macro(const token& name, uint context_order)const{
    return is_macro_name_and_arity(name,0,context_order);
}

bool macro_bank::is_non_zero_arity_macro(const token& name, uint context_order)const{
    if(!could_be_macro_name(name))
        return false;
    else{
        for(const auto& el: content.find(name)->second)
            if(el.second.get_context_order() < context_order)
                return true;
        return false;
    }
}

bool macro_bank::is_macro_name_and_arity(const token& name, uint arity, uint context_order)const{
    const auto it = content.find(name);
    if(it == content.end())
        return false;
    else{
        const auto it2 = it->second.find(arity);
        if(it2 == it->second.end())
            return false;
        else
            return it2->second.get_context_order() < context_order;
    }
}

}
