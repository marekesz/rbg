#ifndef IDENTIFIER_LEAF
#define IDENTIFIER_LEAF

#include"tree_utils.hpp"
#include"parser_helpers.hpp"
#include<memory>

namespace rbg_parser{

class slice_iterator;
class messages_container;
class message;

class identifier_leaf : public expression{
        token name;
        expression_type identifier_type;
        identifier_leaf(slice_iterator&& beginning_position, const token& name);
    public:
        identifier_leaf(void)=delete;
        identifier_leaf(const identifier_leaf&)=default;
        identifier_leaf(identifier_leaf&&)=default;
        identifier_leaf& operator=(identifier_leaf&&)=default;
        identifier_leaf& operator=(const identifier_leaf&)=default;
        virtual ~identifier_leaf(void)=default;
        expression_type get_type(void)const override;
        void type(const typing_machine& m, messages_container& msg)override;
        token get_identifier(void)const override;
        std::vector<token> get_identifiers_sequence(void)const override;
        std::unique_ptr<game_move> get_game_move(void)const override;
        std::unique_ptr<arithmetic_expression> get_arithmetic_expression(void)const override;
        friend parser_result<std::unique_ptr<expression>> parse_identifier_leaf(slice_iterator& it, messages_container& msg);
};

parser_result<std::unique_ptr<expression>> parse_identifier_leaf(slice_iterator& it, messages_container& msg);

}

#endif
