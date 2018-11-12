#ifndef GRAPH
#define GRAPH

#include<vector>
#include<map>
#include<string>
#include"token.hpp"
#include"types.hpp"

namespace rbg_parser{

class unchecked_graph;
typedef std::map<token,uint> edges;

class graph{
        std::vector<std::tuple<token, token, edges>> vertices;
        std::string single_vertex_to_rbg(uint index)const;
    public:
        graph(std::vector<std::tuple<token, token, edges>>&& vertices);
        const token& get_vertex(uint index)const;
        const token& get_starting_piece(uint index)const;
        const edges& get_outgoing_edges(uint index)const;
        uint get_size(void)const;
        std::string to_rbg(bool pretty)const;
};

}

#endif
