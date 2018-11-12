#ifndef TREE_PARSER
#define TREE_PARSER

#include<vector>
#include<map>
#include<memory>
#include<functional>
#include"parser_helpers.hpp"
#include"tree_utils.hpp"

namespace rbg_parser{

class slice_iterator;
class messages_container;
class token;
class typing_machine;

class tree_parser{
        std::vector<std::function<parser_result<std::unique_ptr<expression>>(slice_iterator& it, messages_container& msg)>> leaf_parsers;
        std::vector<std::function<parser_result<suffix>(slice_iterator& it, messages_container& msg)>> suffix_parsers;
        std::map<token_type, operator_info> operators;
        operator_info fallback_operator;
        const typing_machine& t;
        std::map<token_type, bracket_info> brackets;
        suffix parse_suffix(slice_iterator& it, messages_container& msg);
        parser_result<std::unique_ptr<expression>> parse_infix(slice_iterator& it, messages_container& msg);
        parser_result<std::unique_ptr<expression>> parse_bracketed_expression(slice_iterator& it, messages_container& msg);
        std::pair<bool, operator_info> parse_operator(slice_iterator& it, messages_container& msg);
        bool append_one_element(
            std::vector<std::tuple<operator_info, slice_iterator, std::vector<std::unique_ptr<expression>>>>& expressions_stack,
            slice_iterator& it, messages_container& msg);
        std::unique_ptr<expression> flatten_stack(
            std::vector<std::tuple<operator_info, slice_iterator, std::vector<std::unique_ptr<expression>>>>& expressions_stack);
        bool next_is_prefixable(slice_iterator& it, messages_container& msg);
        void prepare_for_prefixable_operator(
            std::vector<std::tuple<operator_info, slice_iterator, std::vector<std::unique_ptr<expression>>>>& expressions_stack,
            slice_iterator& it, messages_container& msg);
        void end_operators_with_higher_priority(
            std::vector<std::tuple<operator_info, slice_iterator, std::vector<std::unique_ptr<expression>>>>& expressions_stack,
            uint priority);
        void prepare_for_prefixable(
            std::vector<std::tuple<operator_info, slice_iterator, std::vector<std::unique_ptr<expression>>>>& expressions_stack,
            slice_iterator& it, messages_container& msg);
    public:
        tree_parser(operator_info fallback_operator, const typing_machine& t);
        tree_parser(void)=delete;
        tree_parser(const tree_parser&)=delete;
        tree_parser& operator=(const tree_parser&)=delete;
        tree_parser(tree_parser&&)=default;
        tree_parser& operator=(tree_parser&&)=default;
        ~tree_parser(void)=default;
        void add_leaf_parser(parser_result<std::unique_ptr<expression>>(*p)(slice_iterator& it, messages_container& msg));
        void add_suffix_parser(parser_result<suffix>(*p)(slice_iterator& it, messages_container& msg));
        void add_operator(token_type representation, operator_info meaning);
        void add_bracket(token_type opening, bracket_info&& br);
        std::unique_ptr<expression> parse_naked_expression(slice_iterator& it, messages_container& msg);
};

}

#endif
