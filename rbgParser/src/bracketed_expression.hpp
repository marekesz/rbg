#ifndef BRACKETED_EXPRESSION
#define BRACKETED_EXPRESSION

#include"tree_utils.hpp"
#include<memory>

namespace rbg_parser{

class bracketed_expression : public expression{
        std::unique_ptr<expression> element;
        bracket_type br;
        expression_type t;
        std::unique_ptr<game_move> append_suffix_if_possible(std::unique_ptr<game_move>&& base_move)const;
    public:
        bracketed_expression(
            slice_iterator&& beginning_position,
            std::unique_ptr<expression> element,
            bracket_type br);
        bracketed_expression(void)=delete;
        bracketed_expression(const bracketed_expression&)=default;
        bracketed_expression(bracketed_expression&&)=default;
        bracketed_expression& operator=(bracketed_expression&&)=default;
        bracketed_expression& operator=(const bracketed_expression&)=default;
        virtual ~bracketed_expression(void)=default;
        expression_type get_type(void)const override;
        void type(const typing_machine& m, messages_container& msg)override;
        std::unique_ptr<arithmetic_expression> get_arithmetic_expression(void)const override;
        std::unique_ptr<game_move> get_game_move(void)const override;
        suffix s = {no_suffix, 0};
};

}

#endif
