//
// Created by ??? on 12.01.18.
//

#ifndef RBGGAMEMANAGER_GAME_DESCRIPTION_H
#define RBGGAMEMANAGER_GAME_DESCRIPTION_H


#include <limits>
#include "name_resolver.h"
#include "../game_nfa/automaton.h"
#include "../game_nfa/game_moves_description.h"
#include "../../rbgParser/src/parsed_game.hpp"
#include "graph_board.h"

// Declarations keep track of declared token ids.
class Declarations {
public:
  explicit Declarations(size_t names_count)
      : bounds_(names_count, 0)
  {}

  void AddPlayer(token_id_t player_id, int bound) {
    players_ids_.push_back(player_id);
    max_player_id_ = std::max(player_id, max_player_id_);
    min_player_id_ = std::min(player_id, min_player_id_);
    bounds_[player_id] = bound;
  }

  void AddVariable(token_id_t variable_id, int bound) {
    variables_ids_.push_back(variable_id);
    max_variable_id_ = std::max(variable_id, max_variable_id_);
    min_variable_id_ = std::min(variable_id, min_variable_id_);
    bounds_[variable_id] = bound;
  }

  void AddPiece(token_id_t piece_id) {
    pieces_ids_.push_back(piece_id);
    max_piece_id_ = std::max(piece_id, max_piece_id_);
    min_piece_id_ = std::min(piece_id, min_piece_id_);
  }

  const std::vector<token_id_t> &players_ids() const {
    return players_ids_;
  }

  const std::vector<token_id_t> &variables_ids() const {
    return variables_ids_;
  }

  const std::vector<token_id_t> &pieces_ids() const {
    return pieces_ids_;
  }

  bool IsPlayer(token_id_t token) const {
    return min_player_id_ <= token && token <= max_player_id_;
  }

  bool IsVariable(token_id_t token) const {
    return min_variable_id_ <= token && token <= max_variable_id_;
  }

  bool IsPiece(token_id_t token) const {
    return min_piece_id_ <= token && token <= max_piece_id_;
  }

  token_id_t max_player_id() const {
    return max_player_id_;
  }

  token_id_t max_variable_id() const {
    return max_variable_id_;
  }

  token_id_t max_piece_id() const {
    return max_piece_id_;
  }

  int bound(token_id_t t) const
  {
    if(min_piece_id_ <= t && t <= max_piece_id_)
    {
      return std::numeric_limits<int>::max();
    }
    return bounds_[t];
  }

private:
  std::vector<token_id_t> players_ids_{};
  token_id_t min_player_id_ = std::numeric_limits<token_id_t >::max();
  token_id_t max_player_id_ = std::numeric_limits<token_id_t >::min();
  std::vector<token_id_t> variables_ids_{};
  token_id_t min_variable_id_ = std::numeric_limits<token_id_t >::max();
  token_id_t max_variable_id_ = std::numeric_limits<token_id_t >::min();
  std::vector<token_id_t> pieces_ids_{};
  token_id_t min_piece_id_ = std::numeric_limits<token_id_t >::max();
  token_id_t max_piece_id_ = std::numeric_limits<token_id_t >::min();

  std::vector<int> bounds_;
};

class GameDescription {
public:
  token_id_t keeper_player_id() const {
    return keeper_id_;
  }

  const std::string text_description() const {
    return game_text_;
  }

  const Declarations &declarations() const {
    return declarations_;
  }

  const NameResolver &resolver() const {
    return resolver_;
  }

  const EdgeResolver &edge_resolver() const{
    return edge_resolver_;
  }

  const VertexResolver &vertex_resolver() const{
    return vertex_resolver_;
  }

  const GameMovesDescription &moves_description() const {
    return moves_description_;
  }

  const GraphBoard &initial_board() const {
    return initial_board_;
  }

  size_t VariablesCount() const {
    return resolver_.NamesCount();
  }

  friend GameDescription
  CreateDescription(const rbg_parser::parsed_game &game);

private:
  /* TODO(???): Make other ways to Create GameDescription */
  GameDescription(NameResolver name_resolver,
                  GameMovesDescription moves_description,
                  EdgeResolver edge_resolver,
                  VertexResolver vertex_resolver,
                  GraphBoard initial_board,
                  Declarations declarations,
                  std::string game_description)
      : resolver_(std::move(name_resolver)),
        moves_description_(std::move(moves_description)),
        edge_resolver_(std::move(edge_resolver)),
        vertex_resolver_(std::move(vertex_resolver)),
        initial_board_(std::move(initial_board)),
        declarations_(std::move(declarations)),
        game_text_(game_description),
        keeper_id_(resolver_.Id(">"))
  {}

  NameResolver resolver_;
  GameMovesDescription moves_description_;

  EdgeResolver edge_resolver_;
  VertexResolver vertex_resolver_;
  GraphBoard initial_board_;

  Declarations declarations_;

  std::string game_text_;

  token_id_t keeper_id_;
};


#endif //RBGGAMEMANAGER_GAME_DESCRIPTION_H
