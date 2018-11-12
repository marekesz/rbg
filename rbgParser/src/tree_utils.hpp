#ifndef TREE_UTILS
#define TREE_UTILS

#include"types.hpp"
#include"slice_iterator.hpp"
#include"token.hpp"
#include<string>
#include<set>
#include<vector>
#include<memory>
#include<cassert>

namespace rbg_parser{

enum bracket_type{
    no_brackets = 0,
    standard_bracket,
    condition_bracket,
    negated_condition_bracket,
    modifier_bracket,
    assignment_bracket,
    comparison_bracket,
    on_bracket,
};

std::string bracket_type_description(bracket_type t);

enum expression_type{
    not_typed_yet = 0,
    error_type,
    // simple:
    integer,
    unsigned_integer,
    variable,
    piece_name,
    player_name,
    edge_name,
    // compund:
    single_assignment,
    assignments_sequence,
    pieces_sequence,
    integer_comparison,
    arithmetics,
    mcheck,
    arithmetic_check,
    shift_move,
    offs_move,
    on_move,
    assignments_move,
    gmove,
};

std::string expression_type_description(expression_type t);
bool is_subtype(expression_type t, expression_type sub_t);

enum operator_type{
    no_operator = 0,
    concatenate,
    add,
    subtract,
    multiply,
    divide,
    separate,
    assign,
    is_less,
    is_less_eq,
    is_greater,
    is_greater_eq,
    is_equal,
    is_nequal,
};

std::string operator_type_description(operator_type t);

struct operator_info{
    operator_type op;
    uint priority;
    bool prefixable;
};

struct bracket_info{
    token_type ending;
    std::string expected_string;
    bracket_type meaning;
};

enum suffix_type{
    no_suffix = 0,
    star_power,
    number_power,
};

std::string suffix_type_description(suffix_type t);

struct suffix{
    suffix_type t;
    uint val;
};

struct possible_operator_interpretation{
    bool arity_matters;
    std::vector<expression_type> types_to_match;
    expression_type resulting_type;
};

struct possible_bracket_interpretation{
    expression_type type_to_match;
    std::set<suffix_type> suffixes_to_match;
    expression_type resulting_type;
};

class typing_machine;
class messages_container;
class message;
class game_move;
class arithmetic_expression;

class expression{
    protected:
        expression(slice_iterator&& beginning_position);
        slice_iterator beginning_position;
    public:
        expression(const expression&)=default;
        expression& operator=(const expression&)=default;
        expression(expression&&)=default;
        expression& operator=(expression&&)=default;
        virtual ~expression(void)=default;
        virtual expression_type get_type(void)const=0;
        virtual const slice_iterator& get_beginning(void)const;
        virtual void type(const typing_machine&, messages_container&)=0;
        virtual uint get_uint(void)const{assert(false);};
        virtual int get_int(void)const{assert(false);};
        virtual token get_identifier(void)const{assert(false);};
        virtual std::vector<token> get_identifiers_sequence(void)const{assert(false);};
        virtual std::vector<std::pair<token, std::unique_ptr<arithmetic_expression>>> get_assignments_sequence(void)const{assert(false);};
        virtual std::pair<token, std::unique_ptr<arithmetic_expression>> get_assignment(void)const{assert(false);};
        virtual std::unique_ptr<arithmetic_expression> get_arithmetic_expression(void)const{assert(false);};
        virtual std::unique_ptr<game_move> get_game_move(void)const{assert(false);};
};

}

#endif
