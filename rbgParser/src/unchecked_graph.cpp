#include"unchecked_graph.hpp"
#include"message.hpp"
#include"declarations.hpp"
#include"slice_iterator.hpp"
#include"graph.hpp"
#include<cassert>

namespace rbg_parser{

void unchecked_graph::add_vertex(const token& name, token&& starting_piece){
    assert(not vertex_exists(name));
    if(vertices.empty())
        first_vertex_name = name;
    vertices.insert(std::make_pair(name, std::make_pair(std::move(starting_piece), neighbors())));
}

bool unchecked_graph::vertex_exists(const token& name)const{
    return vertices.find(name) != vertices.end();
}

void unchecked_graph::add_edge(const token& source_vertex, const token& target_vertex, const slice_iterator& target_position, const token& edge_label){
    assert(vertex_exists(source_vertex) && not edge_exists(source_vertex, edge_label));
    auto prepared_neighbor = std::make_pair(target_position, target_vertex);
    auto& existing_neighbors = vertices.at(source_vertex).second;
    existing_neighbors.insert(std::make_pair(edge_label, std::move(prepared_neighbor)));
}

bool unchecked_graph::edge_exists(const token& source_vertex, const token& edge_label)const{
    if(not vertex_exists(source_vertex))
        return false;
    const auto& existing_neighbors = vertices.at(source_vertex).second;
    return existing_neighbors.find(edge_label) != existing_neighbors.end();
}

std::map<token, uint> unchecked_graph::create_name_number_correspondence(void)const{
    std::map<token, uint> result;
    uint next_number_to_use = 1;
    for(const auto& el: vertices){
        if(el.first == first_vertex_name)
            result.insert(std::make_pair(el.first, 0));
        else
            result.insert(std::make_pair(el.first, next_number_to_use++));
    }
    return result;
}

std::map<token, uint> unchecked_graph::check_and_transform_edges(
    const neighbors& n,
    const std::map<token, uint>& name_number_correspondence,
    messages_container& msg)const{
    std::map<token, uint> result;
    for(const auto& el: n){
        if(name_number_correspondence.find(el.second.second) == name_number_correspondence.end())
            throw msg.build_message(el.second.first.create_call_stack("Target vertex \'"+el.second.first.current(msg).to_string()+"\' was not declared"));
        result.insert(std::make_pair(el.first, name_number_correspondence.at(el.second.second)));
    }
    return result;
}

graph unchecked_graph::build_graph(messages_container& msg)const{
    const std::map<token, uint> name_to_number_correspondence = create_name_number_correspondence();
    std::vector<std::tuple<token, token, edges>> checked_vertices;
    checked_vertices.resize(name_to_number_correspondence.size());
    for(const auto& el: vertices){
        auto checked_edges = check_and_transform_edges(el.second.second, name_to_number_correspondence, msg);
        checked_vertices[name_to_number_correspondence.at(el.first)] = std::make_tuple(el.first, el.second.first, std::move(checked_edges));
    }
    return graph(std::move(checked_vertices));
}

token parse_starting_piece(const declarations& decl, slice_iterator& it, messages_container& msg){
    if(it.current(msg).get_type() != left_square_bracket)
        throw msg.build_message(it.create_call_stack("Expected \'[\', encountered \'"+it.current(msg).to_string()+"\'"));
    it.next(msg);
    if(it.current(msg).get_type() != identifier)
        throw msg.build_message(it.create_call_stack("Expected piece name, encountered \'"+it.current(msg).to_string()+"\'"));
    auto piece_name = it.current(msg);
    const auto& legal_pieces = decl.get_legal_pieces();
    if(legal_pieces.find(piece_name) == legal_pieces.end())
        throw msg.build_message(it.create_call_stack("Piece \'"+piece_name.to_string()+"\' was not declared"));
    it.next(msg);
    if(it.current(msg).get_type() != right_square_bracket)
        throw msg.build_message(it.create_call_stack("Expected \']\', encountered \'"+it.current(msg).to_string()+"\'"));
    it.next(msg);
    return piece_name;
}

bool parse_single_edge(unchecked_graph& g, const token& vertex_name, declarations& decl, slice_iterator& it, messages_container& msg){
    if(it.current(msg).get_type() != identifier)
        throw msg.build_message(it.create_call_stack("Expected edge label, encountered \'"+it.current(msg).to_string()+"\'"));
    auto label_name = it.current(msg);
    if(decl.get_legal_pieces().find(label_name) != decl.get_legal_pieces().end())
        throw msg.build_message(it.create_call_stack("Edge label \'"+it.current(msg).to_string()+"\' was already declared as piece"));
    if(decl.get_legal_players().find(label_name) != decl.get_legal_players().end())
        throw msg.build_message(it.create_call_stack("Edge label \'"+it.current(msg).to_string()+"\' was already declared as player"));
    if(decl.get_legal_variables().find(label_name) != decl.get_legal_variables().end())
        throw msg.build_message(it.create_call_stack("Edge label \'"+it.current(msg).to_string()+"\' was already declared as variable"));
    if(g.edge_exists(vertex_name, label_name))
        throw msg.build_message(it.create_call_stack("Edge label \'"+it.current(msg).to_string()+"\' was already declared"));
    it.next(msg);
    if(it.current(msg).get_type() != colon)
        throw msg.build_message(it.create_call_stack("Expected \':\', encountered \'"+it.current(msg).to_string()+"\'"));
    it.next(msg);
    if(it.current(msg).get_type() != identifier)
        throw msg.build_message(it.create_call_stack("Expected vertex name, encountered \'"+it.current(msg).to_string()+"\'"));
    auto target_name = it.current(msg);
    auto target_position = it;
    g.add_edge(vertex_name, target_name, target_position, label_name);
    decl.add_edge_label(label_name);
    it.next(msg);
    if(it.current(msg).get_type() != comma && it.current(msg).get_type() != right_curly_bracket)
        throw msg.build_message(it.create_call_stack("Expected \',\' or \'}\', encountered \'"+it.current(msg).to_string()+"\'"));
    bool end_of_edges_list = it.current(msg).get_type() == right_curly_bracket;
    it.next(msg);
    return not end_of_edges_list;
}

void parse_edges(unchecked_graph& g, const token& vertex_name, declarations& decl, slice_iterator& it, messages_container& msg){
    if(it.current(msg).get_type() != left_curly_bracket)
        throw msg.build_message(it.create_call_stack("Expected \'{\', encountered \'"+it.current(msg).to_string()+"\'"));
    it.next(msg);
    while(parse_single_edge(g,vertex_name,decl,it,msg));
}

bool parse_vertex(unchecked_graph& g, declarations& decl, slice_iterator& it, messages_container& msg){
    if(not it.has_value())
        return false;
    if(it.current(msg).get_type() != identifier)
        return false;
    auto vertex_name = it.current(msg);
    if(g.vertex_exists(vertex_name))
        throw msg.build_message(it.create_call_stack("Vertex \'"+vertex_name.to_string()+"\' already exists"));
    it.next(msg);
    auto piece_name = parse_starting_piece(decl, it, msg);
    g.add_vertex(vertex_name, std::move(piece_name));
    parse_edges(g,vertex_name,decl,it,msg);
    return true;
}

parser_result<std::unique_ptr<graph_builder>> parse_unchecked_graph(declarations& decl, slice_iterator& it, messages_container& msg){
    unchecked_graph result;
    if(not parse_vertex(result,decl,it,msg))
        return failure<std::unique_ptr<graph_builder>>();
    while(parse_vertex(result,decl,it,msg));
    if(it.has_value())
        msg.add_message(it.create_call_stack("Unexpected tokens at the end of \'board\' segment"));
    return success(std::unique_ptr<graph_builder>(new unchecked_graph(std::move(result))));
}

}
