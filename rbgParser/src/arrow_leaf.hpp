#ifndef ARROW_LEAF
#define ARROW_LEAF

#include"tree_utils.hpp"
#include"parser_helpers.hpp"
#include<memory>

namespace rbg_parser{

class slice_iterator;
class messages_container;
class message;

class arrow_leaf : public expression{
        token next;
        bool has_next;
        arrow_leaf(slice_iterator&& beginning_position, const token& next, bool has_next = true);
    public:
        arrow_leaf(void)=delete;
        arrow_leaf(const arrow_leaf&)=default;
        arrow_leaf(arrow_leaf&&)=default;
        arrow_leaf& operator=(arrow_leaf&&)=default;
        arrow_leaf& operator=(const arrow_leaf&)=default;
        virtual ~arrow_leaf(void)=default;
        expression_type get_type(void)const override;
        void type(const typing_machine& t, messages_container& msg)override;
        std::unique_ptr<game_move> get_game_move(void)const override;
        friend parser_result<std::unique_ptr<expression>> parse_arrow_leaf(slice_iterator& it, messages_container& msg);
};

parser_result<std::unique_ptr<expression>> parse_arrow_leaf(slice_iterator& it, messages_container& msg);

}

#endif
