#include"graph.hpp"

namespace rbg_parser{

graph::graph(std::vector<std::tuple<token, token, edges>>&& vertices):
vertices(std::move(vertices)){
}

std::string graph::single_vertex_to_rbg(uint index)const{
    std::string result = "";
    result += get_vertex(index).to_string();
    result += "[" + get_starting_piece(index).to_string() + "]";
    result += "{";
    bool further_than_first = false;
    const auto& outgoing_edges = get_outgoing_edges(index);
    for(const auto& el: outgoing_edges){
        if(further_than_first)
            result += ",";
        result += el.first.to_string() + ":" + get_vertex(el.second).to_string();
        further_than_first = true;
    }
    result += "}";
    return result;
}

const token& graph::get_vertex(uint index)const{
    return std::get<0>(vertices[index]);
}

const token& graph::get_starting_piece(uint index)const{
    return std::get<1>(vertices[index]);
}

const edges& graph::get_outgoing_edges(uint index)const{
    return std::get<2>(vertices[index]);
}

uint graph::get_size(void)const{
    return vertices.size();
}

std::string graph::to_rbg(bool pretty)const{
    std::string result = (pretty ? "\n" : " ");
    for(uint i=0;i<get_size();++i){
        if(pretty)
            result += "    ";
        result += single_vertex_to_rbg(i);
        if(pretty)
            result += '\n';
    }
    return result;
}

}
