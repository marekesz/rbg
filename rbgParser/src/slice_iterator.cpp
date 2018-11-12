#include<cassert>

#include"slice_iterator.hpp"

namespace rbg_parser{

clipboard::clipboard(void)noexcept:
stored(), // dummy token
up_to_date(false),
frozen(true){
}

void clipboard::paste(const token& t){
    if(up_to_date){
        stored+=(t);
        frozen=false;
    }
    else{
        stored=t;
        frozen=false;
        up_to_date=true;
    }
}

void clipboard::report_end(void){
    frozen = true;
}

void clipboard::report_next_token(void){
    up_to_date = false;
    frozen = true;
}

void clipboard::paste_null_token(void){
    up_to_date = false;
    frozen = false;
}

bool clipboard::contains_full_token(void)const{
    return up_to_date && frozen;
}

bool clipboard::contains_some_token(void)const{
    return up_to_date;
}

bool clipboard::should_paste_into(void)const{
    return !frozen;
}

const token& clipboard::get_current_token()const{
    assert(contains_full_token());
    return stored;
}

backtrace_info::backtrace_info(const slice& s,const std::shared_ptr<backtrace_info>& parent,uint current_begin,uint current_end)noexcept:
parent_slice(parent),
current_slice(s),
parent_begin(current_begin),
parent_end(current_end),
last_should_be_pasted(false){
}

backtrace_info::backtrace_info(const backtrace_info& src)noexcept:
parent_slice(src.parent_slice),
current_slice(src.current_slice),
parent_begin(src.parent_begin),
parent_end(src.parent_end),
last_should_be_pasted(src.last_should_be_pasted){
}

backtrace_info::backtrace_info(backtrace_info&& src)noexcept:
parent_slice(src.parent_slice),
current_slice(std::move(src.current_slice)),
parent_begin(src.parent_begin),
parent_end(src.parent_end),
last_should_be_pasted(src.last_should_be_pasted){
}

backtrace_info::~backtrace_info(void)noexcept{
}

std::vector<slice> backtrace_info::parse_arguments(messages_container& msg,uint current_begin,uint& current_end){
    std::vector<slice> result;
    current_end=current_begin+1;
    if(current_slice.is_end(current_end))
        throw msg.build_message(current_slice.get_token(current_end-1).get_position(),"Unexpected end of macro arguments");
    if(current_slice.get_token(current_end).get_type() != left_round_bracket)
        throw msg.build_message(current_slice.get_token(current_end).get_position(),"Expected \'(\' token, encountered \'"+current_slice.get_token(current_end).to_string()+"\'");
    while(true){
        ++current_end;
        if(current_slice.is_end(current_end))
            throw msg.build_message(current_slice.get_token(current_end-1).get_position(),"Unexpected end of macro arguments");
        uint bracket_balance = 0;
        uint slice_begin = current_end;
        while(!current_slice.is_end(current_end)
            && ((current_slice.get_token(current_end).get_type() != semicolon && current_slice.get_token(current_end).get_type() != right_round_bracket)
                || bracket_balance > 0)){
            if(current_slice.get_token(current_end).get_type() == left_round_bracket)
                ++bracket_balance;
            if(current_slice.get_token(current_end).get_type() == right_round_bracket)
                --bracket_balance;
            ++current_end;
        }
        uint slice_end = current_end;
        result.push_back(current_slice.cut_slice(slice_begin,slice_end));
        if(current_slice.is_end(current_end))
            throw msg.build_message(current_slice.get_token(current_end-1).get_position(),"Unexpected end of macro arguments");
        if(current_slice.get_token(current_end).get_type() == right_round_bracket)
            return result;
    }
}

std::shared_ptr<backtrace_info> backtrace_info::get_parent(void)const{
    return parent_slice;
}

backtrace_info* backtrace_info::get_pointer(void)const{
    return parent_slice.get();
}

uint backtrace_info::get_begin(void)const{
    return parent_begin;
}

uint backtrace_info::get_end(void)const{
    return parent_end;
}

void backtrace_info::set_last_should_be_pasted(void){
    last_should_be_pasted = true;
}

bool backtrace_info::get_last_should_be_pasted(void){
    return last_should_be_pasted;
}

uint backtrace_info::get_context_order(void)const{
    return current_slice.get_context_order();
}

bool backtrace_info::points_at_variable(uint current_end)const{
    return has_value(current_end) && current_slice.should_be_replaced(current(current_end));
}

bool backtrace_info::next_is_tilde(uint current_end)const{
    return !current_slice.is_end(current_end+1) && current_slice.get_token(current_end+1).get_type() == tilde;
}

bool backtrace_info::is_last_token_in_slice(uint current_end)const{
    return has_value(current_end) && current_slice.is_end(current_end+1);
}

const slice& backtrace_info::get_variable_slice(uint current_end)const{
    return current_slice.get_replacement(current(current_end));
}

const token& backtrace_info::current(uint current_end)const{
    assert(!current_slice.is_end(current_end));
    return current_slice.get_token(current_end);
}

bool backtrace_info::has_value(uint current_end)const{
    return !current_slice.is_end(current_end);
}

slice_iterator::slice_iterator(const slice& s, const macro_bank* mcrs)noexcept:
top(),
error_if_end_of_input("Unexpected end of input"),
last_position(s.get_beginning()),
c(),
started(false),
current_begin(0),
current_end(0),
base_context_order(s.get_context_order()),
macros(mcrs){
    std::shared_ptr<backtrace_info> null(nullptr);
    top = std::shared_ptr<backtrace_info>(new backtrace_info(s,null,0,0));
}

void slice_iterator::move_cursor(void){
    if(started)
        ++current_end;
    started = true;
    current_begin = current_end;
}

std::vector<std::pair<uint,std::string>> slice_iterator::create_call_stack(const std::string& details)const{
    std::vector<std::pair<uint,std::string>> result;
    if(!top){
        result.push_back(std::make_pair(last_position,error_if_end_of_input));
        return result;
    }
    result.push_back(std::make_pair(top->current(current_end).get_position(),details));
    backtrace_info* current_element = top->get_pointer();
    uint last_context_order = top->get_context_order();
    uint last_begin = top->get_begin();
    while(current_element){
        const token& t = current_element->current(last_begin);
        if(current_element->get_context_order() > last_context_order)
            result.push_back(std::make_pair(t.get_position(),"in expansion of macro \'"+t.to_string()+"\'"));
        else
            result.push_back(std::make_pair(t.get_position(),"in expansion of \'"+t.to_string()+"\' macro argument"));
        last_context_order = current_element->get_context_order();
        last_begin = current_element->get_begin();
        current_element = current_element->get_pointer();
    }
    return result;
}

bool slice_iterator::has_value(void)const{
    return top || c.contains_full_token();
}

const token& slice_iterator::current(messages_container& msg)const{
    if(!has_value())
        throw msg.build_message(create_call_stack(error_if_end_of_input));
    if(c.contains_full_token())
        return c.get_current_token();
    else
        return top->current(current_end);
}

void slice_iterator::push_next_slice(const slice& s){
    bool last_to_paste = top->next_is_tilde(current_end) || (top->is_last_token_in_slice(current_end) && top->get_last_should_be_pasted());
    std::shared_ptr<backtrace_info> next_top(new backtrace_info(s,top,current_begin,current_end));
    top = next_top;
    current_begin = current_end = 0;
    started = false;
    if(last_to_paste)
        top->set_last_should_be_pasted();
}

void slice_iterator::pop_slice(void){
    if(top->get_last_should_be_pasted() && !c.contains_some_token())
        c.paste_null_token();
    if(!top->get_last_should_be_pasted())
        c.report_end();
    std::shared_ptr<backtrace_info> next_top = top->get_parent();
    current_begin = top->get_begin();
    current_end = top->get_end();
    top = next_top;
    if(top && top->next_is_tilde(current_end))
        move_cursor();
}

bool slice_iterator::handle_standard_token(messages_container& msg){
    try{
        if(top->is_last_token_in_slice(current_end) && top->get_last_should_be_pasted()){
            c.paste(top->current(current_end));
            return false;
        }
        if(top->next_is_tilde(current_end)){
            c.paste(top->current(current_end));
            move_cursor();
            if(top->get_context_order() == base_context_order)
                throw msg.build_message(top->current(current_end).get_position(),"Tokens can be pasted only inside macros");
            if(top->is_last_token_in_slice(current_end))
                throw msg.build_message(top->current(current_end).get_position(),"Tilde at the end of tokens string");
            return false;
        }
        else{
            if(c.should_paste_into())
                c.paste(top->current(current_end));
            c.report_end();
            return true;
        }
    }
    catch(const std::string& m){
        throw msg.build_message(create_call_stack(m));
    }
}

bool slice_iterator::next(messages_container& msg){
    if(has_value() && started)
        last_position = current(msg).get_position();
    c.report_next_token();
    while(true){
        while(top){
            if(c.contains_full_token())
                return true;
            move_cursor();
            if(!top->has_value(current_end))
                pop_slice();
            else
                break;
        }
        if(!top)
            return false;
        if(top->current(current_end).get_type() == tilde)
            throw msg.build_message(top->current(current_end).get_position(),"Unexpected tilde");
        else if(top->points_at_variable(current_end))
            push_next_slice(top->get_variable_slice(current_end));
        else if(macros->could_be_macro_name(top->current(current_end))){
            if(macros->is_zero_arity_macro(top->current(current_end),top->get_context_order()))
                push_next_slice(macros->get_macro_slice(top->current(current_end),std::vector<slice>()));
            else if(macros->is_non_zero_arity_macro(top->current(current_end),top->get_context_order())){
                const token& name = top->current(current_end);
                std::vector<slice> args = top->parse_arguments(msg,current_begin,current_end);
                if(!macros->is_macro_name_and_arity(name,args.size(),top->get_context_order()))
                    throw msg.build_message(name.get_position(),"Macro with given name exists but requires another number of arguments ("+std::to_string(args.size())+" given)");
                push_next_slice(macros->get_macro_slice(name,args));
            }
            else if(handle_standard_token(msg))
                return true;
        }
        else if(handle_standard_token(msg))
            return true;
    }
}

void slice_iterator::swap_parsing_context_string(std::string& context_string){
    std::swap(error_if_end_of_input, context_string);
}

parsing_context_string_guard::parsing_context_string_guard(slice_iterator* iterator, std::string&& current_context_string):
guarded_iterator(iterator){
    guarded_iterator->swap_parsing_context_string(current_context_string);
    previous_context_string = std::move(current_context_string);
}

parsing_context_string_guard::~parsing_context_string_guard(void){
    guarded_iterator->swap_parsing_context_string(previous_context_string);
}

}
