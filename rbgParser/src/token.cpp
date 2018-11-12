#include<cassert>
#include<climits>
#include<algorithm>

#include"token.hpp"

namespace rbg_parser{

token::token(void)noexcept:
position_in_file(0),
type(dummy){}

token::token(uint position, token_type t)noexcept:
position_in_file(position),
type(t){
    if(type == number)
        number_value = 0;
    else if(type == identifier || type == quotation)
        contained_string = std::shared_ptr<std::string>(new std::string);
}

token::token(std::string&& name)noexcept:
position_in_file(0),
type(identifier),
contained_string(new std::string(std::move(name))){
}

std::string tokens_table[] = {
"(",
"{?",
"{!",
"{$",
")",
"[",
"[$",
"]",
"{",
"}",
"~",
"#",
"-",
"+",
"^",
"/",
"*",
",",
";",
":",
"$",
"=",
"->",
"->>",
"!",
"?",
"!=",
"==",
"<",
"<=",
">",
">=",
"players",
"pieces",
"variables",
"rules",
"board",
"hexagon",
"cuboid",
"rectangle",
};

std::string token::to_string(void)const{
    switch(type){
        case left_round_bracket:
        case left_question_bracket:
        case left_exclamation_bracket:
        case left_dollar_bracket:
        case right_round_bracket:
        case left_square_bracket:
        case left_dollar_square_bracket:
        case right_square_bracket:
        case left_curly_bracket:
        case right_curly_bracket:
        case tilde:
        case hash:
        case minus:
        case plus:
        case caret:
        case slash:
        case star:
        case comma:
        case semicolon:
        case colon:
        case dollar:
        case equal:
        case arrow:
        case keeper_arrow:
        case exclamation:
        case question:
        case not_equal:
        case double_equal:
        case less:
        case less_equal:
        case greater:
        case greater_equal:
        case players:
        case pieces:
        case variables:
        case rules:
        case board:
        case hexagon:
        case cuboid:
        case rectangle:
            return tokens_table[type];
        case number:
            return std::to_string(number_value);
        case identifier:
            return *contained_string;
        case quotation:
            return std::string("\"")+*contained_string+"\"";
        case dummy:
            return "dummy";
        default:
            assert(false);
    }
}

const std::string& token::get_string_content(void)const{
    assert(type == quotation or type == identifier);
    return *contained_string;
}

uint token::get_value(void)const{
    assert(type == number);
    return number_value;
}

void token::increase(void){
    assert(type == number);
    ++number_value;
}

token_type token::get_type(void)const{
    return type;
}

bool token::could_be_next_character(char next_character, uint position)const{
    if(type == quotation)
        return true;
    else if(type == number)
        return next_character >= '0' && next_character <= '9';
    else if(type == identifier)
        return (next_character >= 'a' && next_character <= 'z')
            || (next_character >= 'A' && next_character <= 'Z')
            || (position > 0 && next_character >= '0' && next_character <= '9');
    else
        return tokens_table[type].size() > position && tokens_table[type][position] == next_character;
}

bool token::contains_exactly_n_chars(uint n)const{
    if(type == number)
        return true;
    else if(type == identifier)
        return n == contained_string->size();
    else if(type == quotation)
        return n == contained_string->size()+2;
    else
        return n == tokens_table[type].size();
}

void token::take_next_character(char next_character, messages_container& msg){
    if(type == number){
        if((UINT_MAX-(next_character-'0'))/10<=number_value)
            throw msg.build_message(position_in_file,"Too large integer");
        number_value = number_value*10+(next_character-'0');
    }
    else if(type == identifier || type == quotation)
        contained_string->push_back(next_character);
}

uint token::get_position(void)const{
    return position_in_file;
}

void token::reverse_comparison(void){
    switch(type){
        case less:
            type = greater;
            break;
        case less_equal:
            type = greater_equal;
            break;
        case greater:
            type = less;
            break;
        case greater_equal:
            type = less_equal;
            break;
        case not_equal:
        case double_equal:
            break;
        default:
            assert(false);
    }
}

token& token::operator+=(const token& t){
    if(type == minus && t.type == greater)
        type = arrow;
    else if(type == arrow && t.type == greater)
        type = keeper_arrow;
    else if(type == left_curly_bracket && t.type == question)
        type = left_question_bracket;
    else if(type == left_curly_bracket && t.type == exclamation)
        type = left_exclamation_bracket;
    else if(type == left_curly_bracket && t.type == dollar)
        type = left_dollar_bracket;
    else if(type == left_square_bracket && t.type == dollar)
        type = left_dollar_square_bracket;
    else if(type == less && t.type == equal)
        type = less_equal;
    else if(type == greater && t.type == equal)
        type = greater_equal;
    else if(type == equal && t.type == equal)
        type = double_equal;
    else if(type == exclamation && t.type == equal)
        type = not_equal;
    else if(type == identifier && t.type >= players){
        std::string result_string = (*contained_string) + t.to_string();
        for(uint i=players;i<=rectangle;++i)
            if(result_string == tokens_table[i]){
                type = token_type(i);
                contained_string.reset();
            }
        if(type == identifier)
            contained_string = std::shared_ptr<std::string>(new std::string(std::move(result_string)));
    }
    else if(type >= players && type <= rectangle){
        std::string result_string = to_string() + t.to_string();
        bool not_identifier = false;
        for(uint i=players;i<=rectangle;++i)
            if(result_string == tokens_table[i]){
                type = token_type(i);
                not_identifier = true;
            }
        if(!not_identifier){
            type = identifier;
            contained_string = std::shared_ptr<std::string>(new std::string(std::move(result_string)));
        }
    }
    else if(type == number && t.type == number){
        std::string result_string = to_string() + t.to_string();
        number_value = 0;
        for(uint i = 0; i<result_string.size();++i){
            if((UINT_MAX-(result_string[i]-'0'))/10<=number_value)
                throw "Too large integer";
            number_value = number_value*10+(result_string[i]-'0');
        }
    }
    else
        throw "Pasting \'"+to_string()+"\' and \'"+t.to_string()+"\' doesn't create parseable token";
    return *this;
}

std::string to_lower(const std::string& s){
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

bool token::operator==(const token& t)const{
    if(type != t.type)
        return false;
    else if(type == number)
        return number_value == t.number_value;
    else if(type == identifier || type == quotation)
        return *contained_string == *t.contained_string;
    else
        return true;
}

bool token::operator<(const token& t)const{
    if(type != t.type)
        return type < t.type;
    else if(type == number)
        return number_value < t.number_value;
    else if(type == identifier || type == quotation)
        return *contained_string < *t.contained_string;
    else
        return false;
}

std::set<token> create_tokens_set(uint position){
    std::set<token> result;
    for(uint i=0;i<=identifier;++i)
        result.insert(token(position,token_type(i)));
    return result;
}

token create_quotation(uint position){
    return token(position,quotation);
}

bool filter_set_with_character(std::set<token>& current_set, char next_character, uint position, messages_container& msg){
    std::set<token> result;
    for(const auto& el: current_set){
        if(el.could_be_next_character(next_character, position)){
            token temp = el;
            temp.take_next_character(next_character, msg);
            result.insert(std::move(temp));
        }
    }
    bool info = false;
    if(result.size() == 0){
        info = true;
        for(const auto& el: current_set){
            if(el.contains_exactly_n_chars(position))
                result.insert(std::move(el));
        }
    }
    current_set = std::move(result);
    return info;
}

uint seek_end_of_quotation(const std::string& input, uint current_char, token& quote, messages_container& msg){
    uint start_comment = current_char-1;
    while(current_char<input.size() && input[current_char] != '\"'){
        if(input[current_char] == '\n')
            msg.report_new_line(current_char+1);
        quote.take_next_character(input[current_char],msg);
        ++current_char;
    }
    if(current_char == input.size()){
        msg.add_message(start_comment,"Unterminated string");
        return current_char;
    }
    else
        return current_char+1;
}

uint seek_end_of_singleline_comment(const std::string& input, uint current_char, messages_container& msg){
    while(current_char<input.size() && input[current_char] != '\n'){
        ++current_char;
    }
    if(input[current_char] == '\n')
        msg.report_new_line(current_char+1);
    return current_char < input.size() ? current_char+1 : current_char;
}

uint seek_end_of_multiline_comment(const std::string& input, uint current_char, messages_container& msg){
    uint start_comment = current_char-2;
    while(current_char<input.size()-1 && (input[current_char] != '*' || input[current_char+1] != '/')){
            if(input[current_char] == '\n')
                msg.report_new_line(current_char+1);
            ++current_char;
    }
    if(current_char == input.size()-1){
        msg.add_message(start_comment,"Unterminated comment");
        return current_char+1;
    }
    else
        return current_char+2;
}

uint skip_whitespace(const std::string& input, uint current_char, messages_container& msg){
    while(true){
        while(current_char<input.size() && isspace(input[current_char])){
            if(input[current_char] == '\n')
                msg.report_new_line(current_char+1);
            ++current_char;
        }
        if(current_char<input.size()-1 && input[current_char] == '/'){
            if(input[current_char+1] == '/')
                current_char = seek_end_of_singleline_comment(input,current_char+2,msg);
            else if(input[current_char+1] == '*')
                current_char = seek_end_of_multiline_comment(input,current_char+2,msg);
            else
                break;
        }
        else
            break;
    }
    return current_char;
}

std::vector<token> tokenize(const std::string& input, messages_container& msg){
    std::vector<token> result;
    uint current_char = 0;
    while(current_char < input.size()){
        uint token_start = current_char = skip_whitespace(input, current_char, msg);
        if(current_char < input.size()){
            if(input[current_char] == '\"'){
                token quote = create_quotation(current_char);
                current_char = seek_end_of_quotation(input,current_char+1,quote,msg);
                result.push_back(std::move(quote));
            }
            else{
                std::set<token> new_tokens_set = create_tokens_set(current_char);
                while((current_char>=input.size() && !filter_set_with_character(new_tokens_set,' ',current_char-token_start,msg))
                    || !filter_set_with_character(new_tokens_set,input[current_char],current_char-token_start,msg)){
                    ++current_char;
                }
                if(current_char == token_start){
                    msg.add_message(current_char,std::string("Stray \'")+input[current_char]+"\' character in file");
                    ++current_char;
                }
                else
                    result.push_back(std::move(*new_tokens_set.begin()));
            }
        }
    }
    return result;
}

}
