#include"hexagon2D.hpp"
#include"declarations.hpp"
#include"unchecked_graph.hpp"

namespace rbg_parser{

hexagon2D::hexagon2D(slice_iterator&& it):
generator_position(std::move(it)){
}

void hexagon2D::parse_edge_argument(token hexagon2D::*direction, declarations& decl, slice_iterator& it, messages_container& msg){
    this->*direction = parse_edge_name(decl,it,msg);
    if(it.current(msg).get_type() != comma)
        throw msg.build_message(it.create_call_stack("Expected \',\', encountered \'"+it.current(msg).to_string()+"\'"));
    it.next(msg);
}

bool hexagon2D::add_line_if_aligned(std::vector<token>&& line){
    if(starting_pieces.empty()){
        starting_pieces.push_back(std::move(line));
        return true;
    }
    else if(has_peak_line){
        if(starting_pieces.back().size()-1 == line.size()){
            starting_pieces.push_back(std::move(line));
            return true;
        }
        else
            return false;
    }
    else{
        if(starting_pieces.back().size()-1 == line.size()){
            has_peak_line = true;
            peak_line = starting_pieces.size()-1;
            starting_pieces.push_back(std::move(line));
            return true;
        }
        else if(starting_pieces.back().size()+1 == line.size()){
            starting_pieces.push_back(std::move(line));
            return true;
        }
        else
            return false;
    }
}

bool hexagon2D::parse_boardline(declarations& decl, slice_iterator& it, messages_container& msg){
    if(not it.has_value() or it.current(msg).get_type() != left_square_bracket)
        return false;
    auto beginning = it;
    it.next(msg);
    auto pieces_sequence = parse_sequence_with_holes(it,decl.get_legal_pieces(),msg);
    if(not pieces_sequence.is_success())
        throw msg.build_message(it.create_call_stack("Expected pieces sequence, encountered \'"+it.current(msg).to_string()+"\'"));
    if(pieces_sequence.get_value().empty())
        throw msg.build_message(beginning.create_call_stack("Boardline cannot be empty"));
    if(not add_line_if_aligned(pieces_sequence.move_value())){
        if(has_peak_line)
            throw msg.build_message(beginning.create_call_stack("This line should be one square shorter than previous"));
        else
            throw msg.build_message(beginning.create_call_stack("This line should be one square shorter or longer than previous"));
    }
    if(it.current(msg).get_type() != right_square_bracket)
        throw msg.build_message(it.create_call_stack("Expected \']\', encountered \'"+it.current(msg).to_string()+"\'"));
    it.next(msg);
    return true;
}

bool hexagon2D::below_peak_line(uint line_number)const{
    return has_peak_line and line_number > peak_line;
}

bool hexagon2D::above_peak_line(uint line_number)const{
    return not has_peak_line or line_number < peak_line;
}

bool hexagon2D::cell_exists(uint line_number,uint column_number)const{
    return line_number < starting_pieces.size() and column_number < starting_pieces[line_number].size()
       and starting_pieces[line_number][column_number].get_type() == identifier;
}

void hexagon2D::transform_square(uint line_number, uint column_number, unchecked_graph& ug)const{
    token vertex_name("hx"+std::to_string(column_number)+"y"+std::to_string(line_number));
    ug.add_vertex(vertex_name,token(starting_pieces[line_number][column_number]));
    if(line_number>0){
        if((column_number<starting_pieces[line_number].size()-1 or below_peak_line(line_number))
        and cell_exists(line_number-1,column_number+(below_peak_line(line_number)?1:0))){
            token target("hx"+std::to_string(column_number+(below_peak_line(line_number)?1:0))+"y"+std::to_string(line_number-1));
            ug.add_edge(vertex_name,target,generator_position,ne);
        }
        if((column_number>0 or below_peak_line(line_number))
        and cell_exists(line_number-1,column_number-(below_peak_line(line_number)?0:1))){
            token target("hx"+std::to_string(column_number-(below_peak_line(line_number)?0:1))+"y"+std::to_string(line_number-1));
            ug.add_edge(vertex_name,target,generator_position,nw);
        }
    }
    if(line_number<starting_pieces.size()-1){
        if((column_number<starting_pieces[line_number].size()-1 or above_peak_line(line_number))
        and cell_exists(line_number+1,column_number+(above_peak_line(line_number)?1:0))){
            token target("hx"+std::to_string(column_number+(above_peak_line(line_number)?1:0))+"y"+std::to_string(line_number+1));
            ug.add_edge(vertex_name,target,generator_position,se);
        }
        if((column_number>0 or above_peak_line(line_number))
        and cell_exists(line_number+1,column_number-(above_peak_line(line_number)?0:1))){
            token target("hx"+std::to_string(column_number-(above_peak_line(line_number)?0:1))+"y"+std::to_string(line_number+1));
            ug.add_edge(vertex_name,target,generator_position,sw);
        }
    }
    if(column_number<starting_pieces[line_number].size()-1 and cell_exists(line_number,column_number+1)){
        token target("hx"+std::to_string(column_number+1)+"y"+std::to_string(line_number));
        ug.add_edge(vertex_name,target,generator_position,e);
    }
    if(column_number>0 and cell_exists(line_number,column_number-1)){
        token target("hx"+std::to_string(column_number-1)+"y"+std::to_string(line_number));
        ug.add_edge(vertex_name,target,generator_position,w);
    }
}

graph hexagon2D::build_graph(messages_container& msg)const{
    unchecked_graph ug;
    for(uint i=0;i<starting_pieces.size();++i)
        for(uint j=0;j<starting_pieces[i].size();++j)
            if(cell_exists(i,j))
                transform_square(i,j,ug);
    return ug.build_graph(msg);
}

parser_result<std::unique_ptr<graph_builder>> parse_hexagon2D(declarations& decl, slice_iterator& it, messages_container& msg){
    if(not it.has_value() or it.current(msg).get_type() != hexagon)
        return failure<std::unique_ptr<graph_builder>>();
    slice_iterator generator_position = it;
    it.next(msg);
    if(it.current(msg).get_type() != left_round_bracket)
        throw msg.build_message(it.create_call_stack("Expected \'(\', encountered \'"+it.current(msg).to_string()+"\'"));
    it.next(msg);
    hexagon2D result(std::move(generator_position));
    result.parse_edge_argument(&hexagon2D::nw,decl,it,msg);
    result.parse_edge_argument(&hexagon2D::ne,decl,it,msg);
    result.parse_edge_argument(&hexagon2D::e,decl,it,msg);
    result.parse_edge_argument(&hexagon2D::se,decl,it,msg);
    result.parse_edge_argument(&hexagon2D::sw,decl,it,msg);
    result.parse_edge_argument(&hexagon2D::w,decl,it,msg);
    if(not result.parse_boardline(decl,it,msg))
        throw msg.build_message(it.create_call_stack("\'hexagon\' generator requires at least one boardline"));
    while(result.parse_boardline(decl,it,msg));
    if(it.current(msg).get_type() != right_round_bracket)
        throw msg.build_message(it.create_call_stack("Expected \')\', encountered \'"+it.current(msg).to_string()+"\'"));
    if(it.next(msg))
        msg.add_message(it.create_call_stack("Unexpected tokens at the end of \'board\' section"));
    return success(std::unique_ptr<graph_builder>(new hexagon2D(std::move(result))));
}

}
