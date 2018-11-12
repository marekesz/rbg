//
// Created by ??? on 13.01.18.
//

#include <queue>
#include "parser_connection.h"
#include "../game_components/game_description.h"
#include "../game_components/graph_board.h"
#include "game_nfa_creator.h"

EdgeResolver CreateEdgeNamesResolver(const rbg_parser::graph &parsed_graph_board)
{
  std::unordered_set<std::string> edge_names;
  std::vector<bool> visited(parsed_graph_board.get_size(), false);
  std::queue<uint> q;
  for(uint i = 0; i < parsed_graph_board.get_size(); i++) {
    if(!visited[i]) {
      q.push(i);
      visited[i] = true;
      while(!q.empty())
      {
        uint current = q.front();
        q.pop();
        for(const auto& edge : parsed_graph_board.get_outgoing_edges(current))
        {
          edge_names.insert(edge.first.to_string());
          if(!visited[edge.second])
          {
            q.push(edge.second);
            visited[edge.second] = true;
          }
        }
      }
    }
  }
  EdgeResolver result;
  for(const auto& edge_name : edge_names)
  {
    result.AddEdge(edge_name);
  }
  return result;
}

VertexResolver CreateVertexResolver(const rbg_parser::graph &parsed_graph_board)
{
  VertexResolver result(parsed_graph_board.get_size());
  for(uint i = 0; i < parsed_graph_board.get_size(); i++) {
    result.SetName(i,parsed_graph_board.get_vertex(i).to_string());
  }
  return result;
}

GraphBoard CreateGraphBoard(const rbg_parser::graph &parsed_graph_board, const EdgeResolver& edge_resolver,
                            const NameResolver& resolver)
{
  GraphBoard result(parsed_graph_board.get_size());
  for(uint i = 0; i < parsed_graph_board.get_size(); i++)
  {
    for(const auto& edge : parsed_graph_board.get_outgoing_edges(i))
    {
      result.AddEdge(i,edge_resolver.Id(edge.first.to_string()),edge.second);
      result[i] = resolver.Id(parsed_graph_board.get_starting_piece(i).to_string());
    }
  }
  return result;
}

NameResolver CreateResolver(const rbg_parser::declarations &declarations) {
  NameResolver result;
  for (const auto &piece_token : declarations.get_legal_pieces()) {
    std::string piece_name = piece_token.to_string();
    result.AddName(piece_name);
  }
  for (const auto &player_token : declarations.get_legal_players()) {
    std::string player_name = player_token.first.to_string();
    result.AddName(player_name);
  }
  for (const auto &variable_token : declarations.get_legal_variables()) {
    std::string variable_name = variable_token.first.to_string();
    result.AddName(variable_name);
  }
  result.AddName(">");
  return result;
}


GameMovesDescription
CreateMoves(const rbg_parser::game_move &move, const NameResolver &resolver,
            token_id_t piece_id_threshold, const EdgeResolver &edge_resolver, const GraphBoard &board) {
  GameNfaCreator creator(resolver, piece_id_threshold, edge_resolver, board);
  move.accept(creator);
  return creator.ExtractDescription();
}


GameDescription CreateDescription(const rbg_parser::parsed_game &game) {
  NameResolver resolver(CreateResolver(game.get_declarations()));
  EdgeResolver edge_resolver(CreateEdgeNamesResolver(game.get_board()));
  VertexResolver vertex_resolver(CreateVertexResolver(game.get_board()));
  GraphBoard initial_board(CreateGraphBoard(game.get_board(),edge_resolver, resolver));
  GameMovesDescription moves(CreateMoves(*game.get_moves(), resolver,
                                         game.get_declarations().get_legal_pieces().size(), edge_resolver,
                                         initial_board));
  Declarations declarations(
      CreateDeclarations(game.get_declarations(), resolver));
  return std::move(GameDescription(std::move(resolver), std::move(moves),
                                   std::move(edge_resolver),
                                   std::move(vertex_resolver),
                                   std::move(initial_board),
                                   std::move(declarations), game.to_rbg(true)));
}

Declarations CreateDeclarations(const rbg_parser::declarations &declarations,
                                const NameResolver &resolver) {
  Declarations result(resolver.NamesCount());
  for (const auto &piece_token : declarations.get_legal_pieces()) {
    std::string piece_name = piece_token.to_string();
    result.AddPiece(resolver.Id(piece_name));
  }
  for (const auto &player_token : declarations.get_legal_players()) {
    std::string player_name = player_token.first.to_string();
    result.AddPlayer(resolver.Id(player_name), declarations.get_player_bound(player_token.first));
  }
  for (const auto &variable_token : declarations.get_legal_variables()) {
    std::string variable_name = variable_token.first.to_string();
    result.AddVariable(resolver.Id(variable_name), declarations.get_variable_bound(variable_token.first));
  }
  return result;
}
