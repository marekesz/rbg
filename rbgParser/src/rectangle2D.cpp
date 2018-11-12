#include"rectangle2D.hpp"
#include"declarations.hpp"
#include"unchecked_graph.hpp"

namespace rbg_parser{

rectangle2D::rectangle2D(slice_iterator&& it):
generator_position(std::move(it)){
}

void rectangle2D::parse_edge_argument(token rectangle2D::*direction, declarations& decl, slice_iterator& it, messages_container& msg){
    this->*direction = parse_edge_name(decl,it,msg);
    if(it.current(msg).get_type() != comma)
        throw msg.build_message(it.create_call_stack("Expected \',\', encountered \'"+it.current(msg).to_string()+"\'"));
    it.next(msg);
}

bool rectangle2D::add_line_if_aligned(std::vector<token>&& line){
    if(starting_pieces.empty()){
        starting_pieces.push_back(std::move(line));
        return true;
    }
    else if(starting_pieces.back().size() != line.size())
        return false;
    else{
        starting_pieces.push_back(std::move(line));
        return true;
    }
}

bool rectangle2D::parse_boardline(declarations& decl, slice_iterator& it, messages_container& msg){
    if(not it.has_value() or it.current(msg).get_type() != left_square_bracket)
        return false;
    auto beginning = it;
    it.next(msg);
    auto pieces_sequence = parse_sequence_with_holes(it,decl.get_legal_pieces(),msg);
    if(not pieces_sequence.is_success())
        throw msg.build_message(it.create_call_stack("Expected pieces sequence, encountered \'"+it.current(msg).to_string()+"\'"));
    if(not add_line_if_aligned(pieces_sequence.move_value()))
        throw msg.build_message(beginning.create_call_stack("This line differs in length with the previous"));
    if(it.current(msg).get_type() != right_square_bracket)
        throw msg.build_message(it.create_call_stack("Expected \']\', encountered \'"+it.current(msg).to_string()+"\'"));
    it.next(msg);
    return true;
}

void rectangle2D::transform_square(uint line_number, uint column_number, unchecked_graph& ug)const{
    token vertex_name("rx"+std::to_string(column_number)+"y"+std::to_string(line_number));
    ug.add_vertex(vertex_name,token(starting_pieces[line_number][column_number]));
    if(line_number<starting_pieces.size()-1 and cell_exists(line_number+1,column_number)){
        token target("rx"+std::to_string(column_number)+"y"+std::to_string(line_number+1));
        ug.add_edge(vertex_name,target,generator_position,down);
    }
    if(line_number>0 and cell_exists(line_number-1,column_number)){
        token target("rx"+std::to_string(column_number)+"y"+std::to_string(line_number-1));
        ug.add_edge(vertex_name,target,generator_position,up);
    }
    if(column_number<starting_pieces.back().size()-1 and cell_exists(line_number,column_number+1)){
        token target("rx"+std::to_string(column_number+1)+"y"+std::to_string(line_number));
        ug.add_edge(vertex_name,target,generator_position,right);
    }
    if(column_number>0 and cell_exists(line_number,column_number-1)){
        token target("rx"+std::to_string(column_number-1)+"y"+std::to_string(line_number));
        ug.add_edge(vertex_name,target,generator_position,left);
    }
}

bool rectangle2D::cell_exists(uint line_number,uint column_number)const{
    return line_number < starting_pieces.size() and column_number < starting_pieces[line_number].size()
       and starting_pieces[line_number][column_number].get_type() == identifier;
}

graph rectangle2D::build_graph(messages_container& msg)const{
    unchecked_graph ug;
    for(uint i=0;i<starting_pieces.size();++i)
        for(uint j=0;j<starting_pieces[i].size();++j)
            if(cell_exists(i,j))
                transform_square(i,j,ug);
    return ug.build_graph(msg);
}

parser_result<std::unique_ptr<graph_builder>> parse_rectangle2D(declarations& decl, slice_iterator& it, messages_container& msg){
    if(not it.has_value() or it.current(msg).get_type() != rectangle)
        return failure<std::unique_ptr<graph_builder>>();
    auto generator_position = it;
    it.next(msg);
    if(it.current(msg).get_type() != left_round_bracket)
        throw msg.build_message(it.create_call_stack("Expected \'(\', encountered \'"+it.current(msg).to_string()+"\'"));
    it.next(msg);
    rectangle2D result(std::move(generator_position));
    result.parse_edge_argument(&rectangle2D::up,decl,it,msg);
    result.parse_edge_argument(&rectangle2D::down,decl,it,msg);
    result.parse_edge_argument(&rectangle2D::left,decl,it,msg);
    result.parse_edge_argument(&rectangle2D::right,decl,it,msg);
    if(not result.parse_boardline(decl,it,msg))
        throw msg.build_message(it.create_call_stack("\'rectangle\' generator requires at least one boardline"));
    while(result.parse_boardline(decl,it,msg));
    if(it.current(msg).get_type() != right_round_bracket)
        throw msg.build_message(it.create_call_stack("Expected \')\', encountered \'"+it.current(msg).to_string()+"\'"));
    if(it.next(msg))
        msg.add_message(it.create_call_stack("Unexpected tokens at the end of \'board\' section"));
    return success(std::unique_ptr<graph_builder>(new rectangle2D(std::move(result))));
}

}
