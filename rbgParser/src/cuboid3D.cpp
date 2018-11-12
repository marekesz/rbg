#include"cuboid3D.hpp"
#include"declarations.hpp"
#include"unchecked_graph.hpp"

namespace rbg_parser{

cuboid3D::cuboid3D(slice_iterator&& it):
generator_position(std::move(it)){
}

void cuboid3D::parse_edge_argument(token cuboid3D::*direction, declarations& decl, slice_iterator& it, messages_container& msg){
    this->*direction = parse_edge_name(decl,it,msg);
    if(it.current(msg).get_type() != comma)
        throw msg.build_message(it.create_call_stack("Expected \',\', encountered \'"+it.current(msg).to_string()+"\'"));
    it.next(msg);
}

bool cuboid3D::can_add_another_line(void)const{
    if(starting_pieces.size() == 1)
        return true;
    else
        return starting_pieces[0].size() > starting_pieces.back().size();
}

bool cuboid3D::is_line_length_acceptable(uint length)const{
    if(starting_pieces.back().empty())
    {
        if(starting_pieces.size() == 1)
            return true;
        else
            return starting_pieces[0].back().size() == length;
    }
    else
        return starting_pieces.back().back().size() == length;
}

bool cuboid3D::is_layer_aligned(void)const{
    if(starting_pieces.size() == 1)
        return true;
    else
        return starting_pieces[0].size() == starting_pieces.back().size();
}

bool cuboid3D::add_line_if_aligned(std::vector<token>&& line){
    if(is_line_length_acceptable(line.size())){
        starting_pieces.back().push_back(std::move(line));
        return true;
    }
    else
        return false;
}

bool cuboid3D::parse_boardline(declarations& decl, slice_iterator& it, messages_container& msg){
    if(not it.has_value() or it.current(msg).get_type() != left_square_bracket)
        return false;
    auto beginning = it;
    it.next(msg);
    auto pieces_sequence = parse_sequence_with_holes(it,decl.get_legal_pieces(),msg);
    if(not pieces_sequence.is_success())
        throw msg.build_message(it.create_call_stack("Expected pieces sequence, encountered \'"+it.current(msg).to_string()+"\'"));
    if(not can_add_another_line())
        throw msg.build_message(beginning.create_call_stack("Cannot add this boardline, this layer is already as large as the first one"));
    if(not add_line_if_aligned(pieces_sequence.move_value()))
        throw msg.build_message(beginning.create_call_stack("This line differs in length with the previous"));
    if(it.current(msg).get_type() != right_square_bracket)
        throw msg.build_message(it.create_call_stack("Expected \']\', encountered \'"+it.current(msg).to_string()+"\'"));
    it.next(msg);
    return true;
}

bool cuboid3D::parse_layer(declarations& decl, slice_iterator& it, messages_container& msg){
    if(not it.has_value() or it.current(msg).get_type() != left_square_bracket)
        return false;
    it.next(msg);
    starting_pieces.push_back(std::vector<std::vector<token>>());
    if(not parse_boardline(decl,it,msg))
        throw msg.build_message(it.create_call_stack("Expected at least one boardline"));
    while(parse_boardline(decl,it,msg));
    if(not is_layer_aligned())
        throw msg.build_message(it.create_call_stack("Expected more boardlines, layer size is not aligned with the previous"));
    if(it.current(msg).get_type() != right_square_bracket)
        throw msg.build_message(it.create_call_stack("Expected \']\', encountered \'"+it.current(msg).to_string()+"\'"));
    it.next(msg);
    return true;
}

void cuboid3D::transform_square(uint depth, uint line_number, uint column_number, unchecked_graph& ug)const{
    token vertex_name("cx"+std::to_string(column_number)+"y"+std::to_string(line_number)+"z"+std::to_string(depth));
    ug.add_vertex(vertex_name,token(starting_pieces[depth][line_number][column_number]));
    if(depth<starting_pieces.size() and cell_exists(depth+1,line_number,column_number)){
        token target("cx"+std::to_string(column_number)+"y"+std::to_string(line_number)+"z"+std::to_string(depth+1));
        ug.add_edge(vertex_name,target,generator_position,front);
    }
    if(depth>0 and cell_exists(depth-1,line_number,column_number)){
        token target("cx"+std::to_string(column_number)+"y"+std::to_string(line_number)+"z"+std::to_string(depth-1));
        ug.add_edge(vertex_name,target,generator_position,back);
    }
    if(line_number<starting_pieces[depth].size()-1 and cell_exists(depth,line_number+1,column_number)){
        token target("cx"+std::to_string(column_number)+"y"+std::to_string(line_number+1)+"z"+std::to_string(depth));
        ug.add_edge(vertex_name,target,generator_position,down);
    }
    if(line_number>0 and cell_exists(depth,line_number-1,column_number)){
        token target("cx"+std::to_string(column_number)+"y"+std::to_string(line_number-1)+"z"+std::to_string(depth));
        ug.add_edge(vertex_name,target,generator_position,up);
    }
    if(column_number<starting_pieces[depth][line_number].size()-1 and cell_exists(depth,line_number,column_number+1)){
        token target("cx"+std::to_string(column_number+1)+"y"+std::to_string(line_number)+"z"+std::to_string(depth));
        ug.add_edge(vertex_name,target,generator_position,right);
    }
    if(column_number>0 and cell_exists(depth,line_number,column_number-1)){
        token target("cx"+std::to_string(column_number-1)+"y"+std::to_string(line_number)+"z"+std::to_string(depth));
        ug.add_edge(vertex_name,target,generator_position,left);
    }
}

bool cuboid3D::cell_exists(uint depth,uint line_number,uint column_number)const{
    return depth < starting_pieces.size()
       and line_number < starting_pieces[depth].size()
       and column_number < starting_pieces[depth][line_number].size()
       and starting_pieces[depth][line_number][column_number].get_type() == identifier;
}

graph cuboid3D::build_graph(messages_container& msg)const{
    unchecked_graph ug;
    for(uint i=0;i<starting_pieces.size();++i)
        for(uint j=0;j<starting_pieces[i].size();++j)
            for(uint k=0;k<starting_pieces[i][j].size();++k)
                if(cell_exists(i,j,k))
                    transform_square(i,j,k,ug);
    return ug.build_graph(msg);
}

parser_result<std::unique_ptr<graph_builder>> parse_cuboid3D(declarations& decl, slice_iterator& it, messages_container& msg){
    if(not it.has_value() or it.current(msg).get_type() != cuboid)
        return failure<std::unique_ptr<graph_builder>>();
    auto generator_position = it;
    it.next(msg);
    if(it.current(msg).get_type() != left_round_bracket)
        throw msg.build_message(it.create_call_stack("Expected \'(\', encountered \'"+it.current(msg).to_string()+"\'"));
    it.next(msg);
    cuboid3D result(std::move(generator_position));
    result.parse_edge_argument(&cuboid3D::up,decl,it,msg);
    result.parse_edge_argument(&cuboid3D::down,decl,it,msg);
    result.parse_edge_argument(&cuboid3D::left,decl,it,msg);
    result.parse_edge_argument(&cuboid3D::right,decl,it,msg);
    result.parse_edge_argument(&cuboid3D::front,decl,it,msg);
    result.parse_edge_argument(&cuboid3D::back,decl,it,msg);
    if(not result.parse_layer(decl,it,msg))
        throw msg.build_message(it.create_call_stack("\'cuboid\' generator requires at least one rectangular layer"));
    while(result.parse_layer(decl,it,msg));
    if(it.current(msg).get_type() != right_round_bracket)
        throw msg.build_message(it.create_call_stack("Expected \')\', encountered \'"+it.current(msg).to_string()+"\'"));
    if(it.next(msg))
        msg.add_message(it.create_call_stack("Unexpected tokens at the end of \'board\' section"));
    return success(std::unique_ptr<graph_builder>(new cuboid3D(std::move(result))));
}

}
