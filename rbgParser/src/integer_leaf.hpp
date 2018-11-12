#ifndef INTEGER_LEAF
#define INTEGER_LEAF

#include"tree_utils.hpp"
#include"parser_helpers.hpp"
#include<memory>

namespace rbg_parser{

class slice_iterator;
class messages_container;
class message;
class typing_machine;

class integer_leaf : public expression{
        int value;
        integer_leaf(slice_iterator&& beginning_position, int value);
    public:
        integer_leaf(void)=delete;
        integer_leaf(const integer_leaf&)=default;
        integer_leaf(integer_leaf&&)=default;
        integer_leaf& operator=(integer_leaf&&)=default;
        integer_leaf& operator=(const integer_leaf&)=default;
        virtual ~integer_leaf(void)=default;
        expression_type get_type(void)const override;
        void type(const typing_machine&, messages_container&)override;
        uint get_uint(void)const override;
        int get_int(void)const override;
        std::unique_ptr<arithmetic_expression> get_arithmetic_expression(void)const override;
        friend parser_result<std::unique_ptr<expression>> parse_integer_leaf(slice_iterator& it, messages_container& msg);
};

parser_result<std::unique_ptr<expression>> parse_integer_leaf(slice_iterator& it, messages_container& msg);

}

#endif
