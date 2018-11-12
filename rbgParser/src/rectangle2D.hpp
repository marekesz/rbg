#ifndef RECTANGLE2D
#define RECTANGLE2D

#include"token.hpp"
#include"slice_iterator.hpp"
#include"graph_builder.hpp"
#include"parser_helpers.hpp"
#include"graph.hpp"

namespace rbg_parser{

class declarations;
class messages_container;
class message;
class unchecked_graph;

class rectangle2D : public graph_builder{
        token up;
        token down;
        token left;
        token right;
        std::vector<std::vector<token>> starting_pieces;
        slice_iterator generator_position;
        bool add_line_if_aligned(std::vector<token>&& line);
        void parse_edge_argument(token rectangle2D::*direction, declarations& decl, slice_iterator& it, messages_container& msg);
        bool parse_boardline(declarations& decl, slice_iterator& it, messages_container& msg);
        void transform_square(uint line_number,uint column_number, unchecked_graph& ug)const;
        bool cell_exists(uint line_number,uint column_number)const;
    public:
        rectangle2D(slice_iterator&& it);
        rectangle2D(const rectangle2D&)=delete;
        rectangle2D& operator=(const rectangle2D&)=delete;
        rectangle2D(rectangle2D&&)=default;
        rectangle2D& operator=(rectangle2D&&)=default;
        ~rectangle2D(void)override=default;
        graph build_graph(messages_container& msg)const override;
        friend parser_result<std::unique_ptr<graph_builder>> parse_rectangle2D(declarations& decl, slice_iterator& it, messages_container& msg);
};

parser_result<std::unique_ptr<graph_builder>> parse_rectangle2D(declarations& decl, slice_iterator& it, messages_container& msg);

}

#endif
