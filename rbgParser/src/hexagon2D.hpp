#ifndef HEXAGON2D
#define HEXAGON2D

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

class hexagon2D : public graph_builder{
        token nw;
        token ne;
        token e;
        token se;
        token sw;
        token w;
        std::vector<std::vector<token>> starting_pieces;
        bool has_peak_line = false;
        slice_iterator generator_position;
        uint peak_line;
        bool add_line_if_aligned(std::vector<token>&& line);
        void parse_edge_argument(token hexagon2D::*direction, declarations& decl, slice_iterator& it, messages_container& msg);
        bool parse_boardline(declarations& decl, slice_iterator& it, messages_container& msg);
        void transform_square(uint line_number,uint column_number, unchecked_graph& ug)const;
        bool below_peak_line(uint line_number)const;
        bool above_peak_line(uint line_number)const;
        bool cell_exists(uint line_number,uint column_number)const;
    public:
        hexagon2D(slice_iterator&& it);
        hexagon2D(const hexagon2D&)=delete;
        hexagon2D& operator=(const hexagon2D&)=delete;
        hexagon2D(hexagon2D&&)=default;
        hexagon2D& operator=(hexagon2D&&)=default;
        ~hexagon2D(void)override=default;
        graph build_graph(messages_container& msg)const override;
        friend parser_result<std::unique_ptr<graph_builder>> parse_hexagon2D(declarations& decl, slice_iterator& it, messages_container& msg);
};

parser_result<std::unique_ptr<graph_builder>> parse_hexagon2D(declarations& decl, slice_iterator& it, messages_container& msg);

}

#endif
