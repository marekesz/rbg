#ifndef TYPING_MACHINE
#define TYPING_MACHINE

#include<map>
#include"tree_utils.hpp"

namespace rbg_parser{

class declarations;

class typing_machine{
        std::map<operator_type, std::vector<possible_operator_interpretation>> operator_meanings;
        std::map<bracket_type, std::vector<possible_bracket_interpretation>> bracket_meanings;
        std::set<expression_type> suffixable_types;
        const declarations& decls;
        bool operator_types_match(const std::vector<expression_type>& elements, const possible_operator_interpretation& op_int)const;
        bool bracket_types_match(expression_type element, suffix_type s, const possible_bracket_interpretation& br_int)const;
    public:
        typing_machine(const declarations& decls);
        typing_machine(const typing_machine&)=default;
        typing_machine& operator=(const typing_machine&)=default;
        typing_machine(typing_machine&&)=default;
        typing_machine& operator=(typing_machine&&)=default;
        ~typing_machine(void)=default;
        expression_type evaluate_identifier(const token& t)const;
        bool can_be_switch(const token& t)const;
        void add_operator_interpretation(
            operator_type t,
            bool arity_matters,
            std::vector<expression_type>&& types_to_match,
            expression_type result);
        void add_bracket_interpretation(
            bracket_type t,
            expression_type type_to_match,
            std::set<suffix_type>&& suffixes_to_match,
            expression_type result);
        void set_as_suffixable(expression_type t);
        bool can_have_suffix(expression_type t)const;
        expression_type evaluate_operator_sequence(operator_type t, const std::vector<expression_type>& elements)const;
        expression_type evaluate_brackets(bracket_type t, expression_type element, suffix_type s)const;
};

typing_machine prepare_types_for_rbg(const declarations& decls);

}

#endif
