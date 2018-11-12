#include<cassert>

#include"slice.hpp"

namespace rbg_parser{

slice::slice(const slice& src, uint from, uint to)noexcept:
data(src.data),
begin(src.begin+from),
end(src.begin+to),
replacements_owner(false),
replacements(src.replacements),
context_order(src.context_order){
}

slice::slice(const std::vector<token>* d, uint b, uint e, uint co)noexcept:
data(d),
begin(b),
end(e),
replacements_owner(true),
replacements(new std::map<token,slice>()),
context_order(co){
    assert(begin<=end && end <= d->size());
}

slice::slice(const slice& src)noexcept:
data(src.data),
begin(src.begin),
end(src.end),
replacements_owner(true),
replacements(new std::map<token,slice>(*src.replacements)),
context_order(src.context_order){
}

slice::slice(slice&& src)noexcept:
data(src.data),
begin(src.begin),
end(src.end),
replacements_owner(src.replacements_owner),
replacements(src.replacements),
context_order(src.context_order){
    src.replacements = nullptr;
}

slice::~slice(void)noexcept{
    if(replacements_owner)
        delete replacements;
}

slice slice::cut_slice(uint from, uint to)const{
    assert(from<=to && begin+to<=end);
    return slice(*this, from, to);
}

void slice::add_replacement(const token& ident,const slice& value){
    replacements->insert(std::make_pair(ident,std::move(value)));
}

bool slice::should_be_replaced(const token& ident)const{
    return replacements->find(ident) != replacements->end();
}

const slice& slice::get_replacement(const token& ident)const{
    return replacements->find(ident)->second;
}

bool slice::is_empty(void)const{
    return end <= begin;
}

uint slice::get_context_order(void)const{
    return context_order;
}

const token& slice::get_token(uint index)const{
    assert(begin+index<end);
    return (*data)[begin+index];
}

uint slice::get_beginning(void)const{
    if(begin<end)
        return (*data)[begin].get_position();
    else if(end<data->size())
        return (*data)[end].get_position();
    else if(begin>=1)
        return (*data)[begin-1].get_position();
    else
        return 0;
}

bool slice::is_end(uint index)const{
    return begin+index >= end;
}

}
