//
// Created by ??? on 14.01.18.
//

#ifndef RBGGAMEMANAGER_MOVES_INFORMATION_H
#define RBGGAMEMANAGER_MOVES_INFORMATION_H

#include <vector>
#include "../game_nfa/automaton.h"
#include "../utils/fast_bit_vector.h"
#include <unordered_map>
#include "move.h"

class GameState;

class Action;
namespace actions {
  class ConditionCheck;
  class NegatedConditionCheck;
}

struct PerftResult {
  size_t leaf_count;
  size_t node_count;
};

class SearchContext {
public:
  friend class actions::ConditionCheck;
  friend class actions::NegatedConditionCheck;

  SearchContext()
      :
      possible_moves_(nullptr),
      max_depth_(-1),
      last_visited_array_index_(0),
      calculation_state_(nullptr) {
  }

  bool IsUsed() {
    return calculation_state_ != nullptr;
  }

  void clear() {
    last_visited_array_index_ = 0;
  }

  void FindMoves(GameState *state, std::vector<Move> *moves);
  
  std::vector<Move> FindMoves(GameState *state);// TODO remove

  PerftResult FindMovesDeep(GameState *state, size_t perft_depth, ssize_t maximal_depth = -1);

  void FindFirstMove(GameState *state, std::vector<Move> *moves);// TODO remove
  bool ApplyFirstMove(GameState *state);
  bool ApplyFirstRandomMove(GameState *state);

  std::vector<Move> FindFirstMove(GameState *state);// TODO remove

  bool CheckPattern(GameState *state, const fsm::Nfa<const Action *> &nfa);

private:
  bool CheckPattern(const fsm::Nfa<const Action *> &nfa);

  std::size_t NewVisited(const fsm::Nfa<const Action *> &nfa);

  std::size_t NewResultsCache(const fsm::Nfa<const Action *> &nfa);

  void DumpVisited(std::size_t visited_array_index);

  void
  CreateVisitedLayers(std::size_t visited_array_index, std::size_t layer_depth);

  void
  CreateResultLayers(std::size_t results_array_index, std::size_t layer_depth);

  void FastPerft(std::size_t visited_array_index,
                   const fsm::Nfa<const Action *> &nfa,
                   fsm::state_id_t current_state, size_t depth,
                        ssize_t last_block_started=-1, size_t depth_result = 3);

  bool FastPerftKeeper(std::size_t visited_array_index,
                   const fsm::Nfa<const Action *> &nfa,
                   fsm::state_id_t current_state, size_t depth,
                        ssize_t last_block_started=-1, size_t depth_result = 3);



  void FindAllMovesRec(std::size_t visited_array_index,
                       const fsm::Nfa<const Action *> &nfa,
                       fsm::state_id_t current_state, Move *move,
                       ssize_t last_block_started=-1);

  bool FindFirstMoveRec(std::size_t visited_array_index,
                        const fsm::Nfa<const Action *> &nfa,
                        fsm::state_id_t current_state, Move *move,
                        ssize_t last_block_started=-1);

  bool ApplyFirstMoveRec(std::size_t visited_array_index,
                         const fsm::Nfa<const Action *> &nfa,
                         fsm::state_id_t current_state, uint depth,
                         ssize_t last_block_started=-1);

  bool ApplyFirstRandomMoveRec(std::size_t visited_array_index,
                         const fsm::Nfa<const Action *> &nfa,
                         fsm::state_id_t current_state, uint depth,
                         ssize_t last_block_started=-1);

  bool CheckPlay(std::size_t visited_array_index,
                 const fsm::Nfa<const Action *> &nfa,
                 fsm::state_id_t current_state, std::size_t depth,
                 ssize_t last_block_started=-1);

  std::size_t VisitedIndex(const fsm::Nfa<const Action *> &nfa,
                           fsm::state_id_t current_state) const;

  std::vector<Move> *possible_moves_;
  ssize_t max_depth_;
  size_t leaf_count_;
  size_t node_count_;

  std::vector<std::vector<FastBitVector> > visited_;

  std::size_t last_visited_array_index_;

  GameState *calculation_state_;

};


#endif //RBGGAMEMANAGER_MOVES_INFORMATION_H
