#ifndef INTERNAL_NODE
#define INTERNAL_NODE

#include"tree_utils.hpp"
#include<memory>
#include<vector>

namespace rbg_parser{

class internal_node : public expression{
        std::vector<std::unique_ptr<expression>> elements;
        operator_type op;
        expression_type t;
    public:
        internal_node(
            slice_iterator&& beginning_position,
            std::vector<std::unique_ptr<expression>>&& elements,
            operator_type op);
        internal_node(void)=delete;
        internal_node(const internal_node&)=default;
        internal_node(internal_node&&)=default;
        internal_node& operator=(internal_node&&)=default;
        internal_node& operator=(const internal_node&)=default;
        virtual ~internal_node(void)=default;
        expression_type get_type(void)const override;
        std::vector<token> get_identifiers_sequence(void)const override;
        std::vector<std::pair<token, std::unique_ptr<arithmetic_expression>>> get_assignments_sequence(void)const override;
        std::pair<token, std::unique_ptr<arithmetic_expression>> get_assignment(void)const override;
        std::unique_ptr<arithmetic_expression> get_arithmetic_expression(void)const override;
        std::unique_ptr<game_move> get_game_move(void)const override;
        void type(const typing_machine& m, messages_container& msg)override;
};

}

#endif
