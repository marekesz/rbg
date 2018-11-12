#ifndef UNCHECKED_GRAPH
#define UNCHECKED_GRAPH

#include<map>
#include<memory>
#include"token.hpp"
#include"slice_iterator.hpp"
#include"graph_builder.hpp"
#include"parser_helpers.hpp"

namespace rbg_parser{

class declarations;
class messages_container;
class message;
class graph;

class unchecked_graph : public graph_builder{
        typedef std::pair<slice_iterator, token> neighbor_name;
        typedef std::map<token, neighbor_name> neighbors;
        std::map<token, std::pair<token, neighbors>> vertices;
        token first_vertex_name;
        std::map<token, uint> create_name_number_correspondence(void)const;
        std::map<token, uint> check_and_transform_edges(
            const neighbors& n,
            const std::map<token, uint>& name_number_correspondence,
            messages_container& msg)const;
    public:
        unchecked_graph(void)=default;
        unchecked_graph(const unchecked_graph&)=delete;
        unchecked_graph& operator=(const unchecked_graph&)=delete;
        unchecked_graph(unchecked_graph&&)=default;
        unchecked_graph& operator=(unchecked_graph&&)=default;
        ~unchecked_graph(void)override=default;
        void add_vertex(const token& name, token&& starting_piece);
        bool vertex_exists(const token& name)const;
        void add_edge(const token& source_vertex, const token& target_vertex, const slice_iterator& target_position, const token& edge_label);
        bool edge_exists(const token& source_vertex, const token& edge_label)const;
        graph build_graph(messages_container& msg)const override;
};

parser_result<std::unique_ptr<graph_builder>> parse_unchecked_graph(declarations& decl, slice_iterator& it, messages_container& msg);

}

#endif
