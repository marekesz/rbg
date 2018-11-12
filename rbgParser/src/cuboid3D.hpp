#ifndef CUBOID3D
#define CUBOID3D

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

class cuboid3D : public graph_builder{
        token up;
        token down;
        token left;
        token right;
        token front;
        token back;
        std::vector<std::vector<std::vector<token>>> starting_pieces;
        slice_iterator generator_position;
        bool add_line_if_aligned(std::vector<token>&& line);
        void parse_edge_argument(token cuboid3D::*direction, declarations& decl, slice_iterator& it, messages_container& msg);
        bool parse_boardline(declarations& decl, slice_iterator& it, messages_container& msg);
        bool parse_layer(declarations& decl, slice_iterator& it, messages_container& msg);
        void transform_square(uint depth,uint line_number,uint column_number, unchecked_graph& ug)const;
        bool cell_exists(uint depth,uint line_number,uint column_number)const;
        bool is_line_length_acceptable(uint length)const;
        bool can_add_another_line(void)const;
        bool is_layer_aligned(void)const;
    public:
        cuboid3D(slice_iterator&& it);
        cuboid3D(const cuboid3D&)=delete;
        cuboid3D& operator=(const cuboid3D&)=delete;
        cuboid3D(cuboid3D&&)=default;
        cuboid3D& operator=(cuboid3D&&)=default;
        ~cuboid3D(void)override=default;
        graph build_graph(messages_container& msg)const override;
        friend parser_result<std::unique_ptr<graph_builder>> parse_cuboid3D(declarations& decl, slice_iterator& it, messages_container& msg);
};

parser_result<std::unique_ptr<graph_builder>> parse_cuboid3D(declarations& decl, slice_iterator& it, messages_container& msg);

}

#endif
