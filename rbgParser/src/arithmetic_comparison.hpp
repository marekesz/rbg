#ifndef ARITHMETIC_COMPARISON
#define ARITHMETIC_COMPARISON

#include"game_move.hpp"
#include"token.hpp"
#include"abstract_dispatcher.hpp"
#include"arithmetic_expression.hpp"
#include<memory>

namespace rbg_parser{

enum kind{
    eq,
    neq,
    le,
    leq,
    ge,
    geq
};

class arithmetic_comparison : public game_move{
        std::unique_ptr<arithmetic_expression> left_side;
        kind kind_of_comparison;
        std::unique_ptr<arithmetic_expression> right_side;
        arithmetic_comparison(void)=default;
    public:
        arithmetic_comparison(
            std::unique_ptr<arithmetic_expression> left_side,
            kind kind_of_comparison,
            std::unique_ptr<arithmetic_expression> right_side);
        ~arithmetic_comparison(void)override=default;
        arithmetic_comparison(arithmetic_comparison&&)=default;
        arithmetic_comparison(const arithmetic_comparison&)=delete;
        arithmetic_comparison& operator=(const arithmetic_comparison&)=delete;
        arithmetic_comparison& operator=(arithmetic_comparison&&)=default;
        void accept(abstract_dispatcher& dispatcher)const override;
        const arithmetic_expression* get_left_side(void)const;
        kind get_kind_of_comparison(void)const;
        const arithmetic_expression* get_right_side(void)const;
        std::unique_ptr<game_move> simplify(void)override;
        uint priority(void)const override{return 3;};
        std::string to_rbg(uint)const override;
        std::string to_rbg()const override;
        std::unique_ptr<game_move> flatten(void)override;
        std::unique_ptr<game_move> copy(void)const override;
};

}

#endif
