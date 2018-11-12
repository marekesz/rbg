//
// Created by ??? on 14.01.18.
//

#include "search_context.h"
#include "game_state.h"
#include <algorithm>

// TODO(???): Add blocks of modifiers.

void SearchContext::FindAllMovesRec(size_t visited_array_index,
                                    const fsm::Nfa<const Action *> &nfa,
                                    fsm::state_id_t current_state, Move *move,
                                    ssize_t last_block_started) {
  const uint index = VisitedIndex(nfa, current_state);
  const size_t depth = move->blocks().size();
  if (visited_[visited_array_index][depth][index])
    return;
  visited_[visited_array_index][depth].set(index);
  for (const auto &transition : nfa[current_state].transitions()) {
    if(transition.letter()->type() == ActionType::kShiftTableType)
    {
      const auto shift_table = static_cast<const actions::ShiftTable*>(transition.letter());
      const auto previous_pos = calculation_state_->pos();
      for(auto next_pos : shift_table->table()[previous_pos])
      {
        calculation_state_->SetPos(next_pos);
        FindAllMovesRec(visited_array_index, nfa, transition.target(), move,-1);
      }
      calculation_state_->SetPos(previous_pos);
      continue;
    }
    if (transition.letter()->IsSwitch()) {
        if (last_block_started != transition.letter()->index()) {
          move->AddBlock(calculation_state_->pos(),transition.letter()->index());
          possible_moves_->push_back(*move);
          move->PopBlock();
        } else possible_moves_->push_back(*move);
        continue;
    }
    ActionResult result = transition.letter()->Apply(calculation_state_);
    if (result) {
      if (transition.letter()->IsModifier()) {
        if (last_block_started != transition.letter()->index()) {
          move->AddBlock(calculation_state_->pos(),
                         transition.letter()->index());
          CreateVisitedLayers(visited_array_index, depth + 1);
        }
        FindAllMovesRec(visited_array_index, nfa, transition.target(), move,
                        transition.letter()->index());
        if (last_block_started != transition.letter()->index()) {
          move->PopBlock();
        }
      } else {
          FindAllMovesRec(visited_array_index, nfa, transition.target(), move,-1);
      }
    }
    transition.letter()->Revert(calculation_state_, result);
  }
}

size_t
SearchContext::VisitedIndex(const fsm::Nfa<const Action *> &nfa,
                            fsm::state_id_t current_state) const {
  return (calculation_state_->pos()) * nfa.StateCount() + current_state;
}

size_t SearchContext::NewVisited(const fsm::Nfa<const Action *> &nfa) {
  if (last_visited_array_index_ < visited_.size()) {
    visited_[last_visited_array_index_].front().resize(
        std::max(visited_[last_visited_array_index_].front().size(),
                 calculation_state_->board().size() *
                 nfa.StateCount()));
    visited_[last_visited_array_index_].front().reset();
    return last_visited_array_index_++;
  }
  visited_.emplace_back();
  visited_.back().emplace_back(
      calculation_state_->board().size() *
      nfa.StateCount(), false);
  last_visited_array_index_ = visited_.size();
  return visited_.size() - 1;
}

bool SearchContext::CheckPlay(size_t visited_array_index,
                              const fsm::Nfa<const Action *> &nfa,
                              fsm::state_id_t current_state, size_t depth,
                              ssize_t last_block_started) {
  if (current_state == nfa.final())
    return true;
  const uint index = VisitedIndex(nfa, current_state);
  if (visited_[visited_array_index][depth][index])
    return false;
  visited_[visited_array_index][depth].set(index);
  for (const auto &transition : nfa[current_state].transitions()) {
    if(transition.letter()->type() == ActionType::kShiftTableType)
    {
      const auto shift_table = static_cast<const actions::ShiftTable*>(transition.letter());
      const auto previous_pos = calculation_state_->pos();
      for(auto next_pos : shift_table->table()[previous_pos])
      {
        calculation_state_->SetPos(next_pos);
        if (CheckPlay(visited_array_index, nfa,
                      transition.target(), depth,-1)) {
          calculation_state_->SetPos(previous_pos);
          return true;
        }
      }
      calculation_state_->SetPos(previous_pos);
      continue;
    }
    ActionResult result = transition.letter()->Apply(calculation_state_);
    bool eval = false;
    if (result) {
      if (transition.letter()->IsModifier()) {
        size_t new_depth = depth;
        if (last_block_started != transition.letter()->index()) {
          CreateVisitedLayers(visited_array_index, depth + 1);
          new_depth = depth + 1;
        }
        if (CheckPlay(visited_array_index, nfa,
                      transition.target(), new_depth, transition.letter()->index()))
          eval = true;
      } else if (CheckPlay(visited_array_index, nfa,
                           transition.target(), depth, -1))
        eval = true;
    }
    transition.letter()->Revert(calculation_state_, result);
    if (eval)
      return true;
  }
  return false;
}

void
SearchContext::FindMoves(GameState *state, std::vector<Move> *moves) {
  calculation_state_ = state;
  possible_moves_ = moves;
  size_t visited_index = NewVisited(
      state->description().moves_description().nfa());
  Move empty;
  FindAllMovesRec(visited_index, state->description().moves_description().nfa(),
                  state->nfa_state(), &empty);
  DumpVisited(visited_index);
  calculation_state_ = nullptr;
  //return std::move(possible_moves_);
}

std::vector<Move>
SearchContext::FindMoves(GameState *state) {
  calculation_state_ = state;
  std::vector<Move> moves;
  possible_moves_ = &moves;
  size_t visited_index = NewVisited(
      state->description().moves_description().nfa());
  Move empty;
  FindAllMovesRec(visited_index, state->description().moves_description().nfa(),
                  state->nfa_state(), &empty);
  DumpVisited(visited_index);
  calculation_state_ = nullptr;
  return std::move(moves);
}

bool SearchContext::CheckPattern(GameState *state,
                                 const fsm::Nfa<const Action *> &nfa) {
  calculation_state_ = state;
  bool result = CheckPattern(nfa);
  calculation_state_ = nullptr;
  return result;
}

void SearchContext::CreateVisitedLayers(size_t visited_array_index,
                                        size_t layer_depth) {
  if (visited_[visited_array_index].size() > layer_depth) {
    visited_[visited_array_index][layer_depth].resize(
        visited_[visited_array_index].front().size());
    visited_[visited_array_index][layer_depth].reset();
    return;
  }
  while (visited_[visited_array_index].size() <= layer_depth) {
    visited_[visited_array_index].emplace_back(
        visited_[visited_array_index].front().size(), false);
  }
}

void SearchContext::DumpVisited(std::size_t visited_array_index) {
  if (visited_array_index == last_visited_array_index_ - 1) {
    last_visited_array_index_--;
  }
}

void
SearchContext::FindFirstMove(GameState *state, std::vector<Move> *moves) {
  calculation_state_ = state;
  possible_moves_ = moves;
  size_t visited_index = NewVisited(
      state->description().moves_description().nfa());
  Move empty;
  FindFirstMoveRec(visited_index,
                   state->description().moves_description().nfa(),
                   state->nfa_state(), &empty);
  DumpVisited(visited_index);
  calculation_state_ = nullptr;
  //return std::move(possible_moves_);
}

std::vector<Move>
SearchContext::FindFirstMove(GameState *state) {
  calculation_state_ = state;
  std::vector<Move> moves;
  possible_moves_ = &moves;
  size_t visited_index = NewVisited(
      state->description().moves_description().nfa());
  Move empty;
  FindFirstMoveRec(visited_index,
                   state->description().moves_description().nfa(),
                   state->nfa_state(), &empty);
  DumpVisited(visited_index);
  calculation_state_ = nullptr;
  return std::move(moves);
}

bool SearchContext::FindFirstMoveRec(std::size_t visited_array_index,
                                     const fsm::Nfa<const Action *> &nfa,
                                     fsm::state_id_t current_state, Move *move,
                                     ssize_t last_block_started) {
  const uint index = VisitedIndex(nfa, current_state);
  const size_t depth = move->blocks().size();
  if (visited_[visited_array_index][depth][index])
    return false;
  visited_[visited_array_index][depth].set(index);
  if ((ssize_t) depth == max_depth_)
    return false;
  for (const auto &transition : nfa[current_state].transitions()) {
    if(transition.letter()->type() == ActionType::kShiftTableType)
    {
      auto shift_table = static_cast<const actions::ShiftTable*>(transition.letter());
      auto previous_pos = calculation_state_->pos();
      for(auto next_pos : shift_table->table()[previous_pos]) {
        calculation_state_->SetPos(next_pos);
        if (FindFirstMoveRec(visited_array_index, nfa, transition.target(), move, -1)) {
          calculation_state_->SetPos(previous_pos);
          return true;
        }
      }
      calculation_state_->SetPos(previous_pos);
      continue;
    }
    if (transition.letter()->IsSwitch()) {
      if (last_block_started != transition.letter()->index()) {
        move->AddBlock(calculation_state_->pos(),
                       transition.letter()->index());
        possible_moves_->push_back(*move);
        move->PopBlock();
      } else possible_moves_->push_back(*move);
      return true;
    }
    ActionResult result = transition.letter()->Apply(calculation_state_);
    bool rec_result = false;
    if (result) {
      if (transition.letter()->IsModifier()) {
        if (last_block_started != transition.letter()->index()) {
          move->AddBlock(calculation_state_->pos(),
                         transition.letter()->index());
          CreateVisitedLayers(visited_array_index, depth + 1);
        }
        rec_result = FindFirstMoveRec(visited_array_index, nfa,
                                      transition.target(), move, transition.letter()->index());
        if (last_block_started != transition.letter()->index()) {
          move->PopBlock();
        }
      } else
        rec_result = FindFirstMoveRec(visited_array_index, nfa,
                                      transition.target(), move, -1);
    }
    transition.letter()->Revert(calculation_state_, result);
    if (rec_result)
      return true;
  }
  return false;
}

bool SearchContext::ApplyFirstMove(GameState *state) {
  calculation_state_ = state;
  size_t visited_index = NewVisited(
      state->description().moves_description().nfa());
  bool result = ApplyFirstMoveRec(visited_index,
                                  state->description().moves_description().nfa(),
                                  state->nfa_state(), 0);
  DumpVisited(visited_index);
  calculation_state_ = nullptr;
  return result;
}

bool SearchContext::ApplyFirstMoveRec(std::size_t visited_array_index,
                                     const fsm::Nfa<const Action *> &nfa,
                                     fsm::state_id_t current_state, uint depth,
                                     ssize_t last_block_started) {
  const uint index = VisitedIndex(nfa, current_state);
  visited_[visited_array_index][depth].set(index);  
  for (const auto &transition : nfa[current_state].transitions()) {
    if(transition.letter()->type() == ActionType::kShiftTableType)
    {
      auto shift_table = static_cast<const actions::ShiftTable*>(transition.letter());
      auto previous_pos = calculation_state_->pos();
      for(auto next_pos : shift_table->table()[previous_pos]) {
        calculation_state_->SetPos(next_pos);
        if (ApplyFirstMoveRec(visited_array_index, nfa, transition.target(), depth, last_block_started))
          return true;
      }
      calculation_state_->SetPos(previous_pos);
      continue;
    }
    if (transition.letter()->IsSwitch()) {
      transition.letter()->Apply(calculation_state_);
      calculation_state_->current_state_ = calculation_state_->parent_.moves_description().CorrespondingState(transition.letter()->index());
       while (
          calculation_state_->parent_.moves_description().nfa()[calculation_state_->current_state_].transitions().size() == 1 &&
          calculation_state_->parent_.moves_description().nfa()[calculation_state_->current_state_].transitions().front().letter()->index() == transition.letter()->index()) {
        calculation_state_->current_state_ = calculation_state_->parent_.moves_description().nfa()[calculation_state_->current_state_].transitions().front().target();
      }
      return true;
    }
    ActionResult result = transition.letter()->Apply(calculation_state_);
    if (result) {
      if (transition.letter()->IsModifier()) {
        uint new_depth;
        if (last_block_started != transition.letter()->index()) {
          new_depth = depth+1;
          CreateVisitedLayers(visited_array_index, new_depth);
        } else new_depth = depth;
        if (ApplyFirstMoveRec(visited_array_index, nfa,
                              transition.target(), new_depth, transition.letter()->index()))
          return true;
      } else {
        if (ApplyFirstMoveRec(visited_array_index, nfa,
                              transition.target(), depth, last_block_started))
          return true;
        }
    }
    transition.letter()->Revert(calculation_state_, result);
  }
  return false;
}

bool SearchContext::ApplyFirstRandomMove(GameState *state) {
  calculation_state_ = state;
  size_t visited_index = NewVisited(
      state->description().moves_description().nfa());
  bool result = ApplyFirstRandomMoveRec(visited_index,
                                  state->description().moves_description().nfa(),
                                  state->nfa_state(), 0);
  DumpVisited(visited_index);
  calculation_state_ = nullptr;
  return result;
}

bool SearchContext::ApplyFirstRandomMoveRec(std::size_t visited_array_index,
                                     const fsm::Nfa<const Action *> &nfa,
                                     fsm::state_id_t current_state, uint depth,
                                     ssize_t last_block_started) {
  const uint index = VisitedIndex(nfa, current_state);
  visited_[visited_array_index][depth].set(index);
  boost::container::small_vector<uint,16> order_trans;
    order_trans.resize(nfa[current_state].transitions().size());
  for (uint i = 0; i < order_trans.size(); i++) order_trans[i] = i;
  std::random_shuffle(order_trans.begin(), order_trans.end());
  for (uint i = 0; i < order_trans.size(); i++) {
    const auto &transition = nfa[current_state].transitions()[order_trans[i]];
    if(transition.letter()->type() == ActionType::kShiftTableType)
    {
      auto shift_table = static_cast<const actions::ShiftTable*>(transition.letter());
      auto previous_pos = calculation_state_->pos();
      boost::container::small_vector<uint,16> order_table;
      order_table.resize(shift_table->table()[previous_pos].size());
      for (uint j = 0; j < order_table.size(); j++) order_table[j] = j;
      std::random_shuffle(order_table.begin(), order_table.end());
      for (uint j = 0; j < order_table.size(); j++) {
        calculation_state_->SetPos(shift_table->table()[previous_pos][order_table[j]]);
        if (ApplyFirstRandomMoveRec(visited_array_index, nfa, transition.target(), depth, -1))
          return true;
      }
      calculation_state_->SetPos(previous_pos);
      continue;
    }
    if (transition.letter()->IsSwitch()) {
      transition.letter()->Apply(calculation_state_);
      calculation_state_->current_state_ = calculation_state_->parent_.moves_description().CorrespondingState(transition.letter()->index());
       while (
          calculation_state_->parent_.moves_description().nfa()[calculation_state_->current_state_].transitions().size() == 1 &&
          calculation_state_->parent_.moves_description().nfa()[calculation_state_->current_state_].transitions().front().letter()->index() == transition.letter()->index()) {
        calculation_state_->current_state_ = calculation_state_->parent_.moves_description().nfa()[calculation_state_->current_state_].transitions().front().target();
      }
      return true;
    }
    ActionResult result = transition.letter()->Apply(calculation_state_);
    if (result) {
      if (transition.letter()->IsModifier()) {
        uint new_depth;
        if (last_block_started != transition.letter()->index()) {
          new_depth = depth+1;
          CreateVisitedLayers(visited_array_index, new_depth);
        } else new_depth = depth;
        if (ApplyFirstRandomMoveRec(visited_array_index, nfa,
                              transition.target(), new_depth, transition.letter()->index()))
          return true;
      } else {
        if (ApplyFirstRandomMoveRec(visited_array_index, nfa,
                              transition.target(), depth, -1))
          return true;
        }
    }
    transition.letter()->Revert(calculation_state_, result);
  }
  return false;
}

bool SearchContext::CheckPattern(const fsm::Nfa<const Action *> &nfa) {
  const size_t visited_index = NewVisited(nfa);
  bool result = CheckPlay(visited_index, nfa, nfa.initial(), 0);
  DumpVisited(visited_index);
  return result;
}

void SearchContext::FastPerft(std::size_t visited_array_index, const fsm::Nfa<const Action *> &nfa,
                                fsm::state_id_t current_state, size_t depth, ssize_t last_block_started, size_t perft_depth) {
  const uint index = VisitedIndex(nfa, current_state);
  if (visited_[visited_array_index][depth][index])
    return;
  visited_[visited_array_index][depth].set(index);
  if ((ssize_t) depth == max_depth_)
    return;
  for (const auto &transition : nfa[current_state].transitions()) {
    if(transition.letter()->type() == ActionType::kShiftTableType)
    {
      auto shift_table = static_cast<const actions::ShiftTable*>(transition.letter());
      auto previous_pos = calculation_state_->pos();
      for(auto next_pos : shift_table->table()[previous_pos])
      {
        calculation_state_->SetPos(next_pos);
        FastPerft(visited_array_index, nfa, transition.target(), depth,-1 , perft_depth); 
      }
      calculation_state_->SetPos(previous_pos);
      continue;
    }
    ActionResult result = transition.letter()->Apply(calculation_state_);
    if (result) {
      if (transition.letter()->IsModifier()) {
        size_t new_depth = depth;
        if (last_block_started != transition.letter()->index()) {
          new_depth = depth + 1;
          CreateVisitedLayers(visited_array_index, depth + 1);
        }
        if (transition.letter()->IsSwitch()) {
          if (calculation_state_->player() == calculation_state_->description().keeper_player_id()) {
            if (!FastPerftKeeper(visited_array_index, nfa, transition.target(), new_depth, transition.letter()->index(), perft_depth)) {
              node_count_++;
              if (perft_depth == 1) leaf_count_++;
            }
          }
          else {
          node_count_++;
          if (perft_depth > 1) {
            FastPerft(visited_array_index, nfa, transition.target(), new_depth, transition.letter()->index(), perft_depth - 1);
          }
          else {
            leaf_count_++;
          }
          }
        }
        else {
          FastPerft(visited_array_index, nfa, transition.target(), new_depth, transition.letter()->index(), perft_depth); 
        }
      } else {
        FastPerft(visited_array_index, nfa, transition.target(), depth, -1, perft_depth); 
      }
    }
    transition.letter()->Revert(calculation_state_, result);
   
  }
}

bool SearchContext::FastPerftKeeper(std::size_t visited_array_index, const fsm::Nfa<const Action *> &nfa,
                                fsm::state_id_t current_state, size_t depth, ssize_t last_block_started, size_t perft_depth) {
  const uint index = VisitedIndex(nfa, current_state);
  if (visited_[visited_array_index][depth][index])
    return false;
  visited_[visited_array_index][depth].set(index);
  if ((ssize_t) depth == max_depth_)
    return false;
  for (const auto &transition : nfa[current_state].transitions()) {
    if(transition.letter()->type() == ActionType::kShiftTableType)
    {
      auto shift_table = static_cast<const actions::ShiftTable*>(transition.letter());
      auto previous_pos = calculation_state_->pos();
      for(auto next_pos : shift_table->table()[previous_pos])
      {
        calculation_state_->SetPos(next_pos);
        auto rec_result = FastPerftKeeper(visited_array_index, nfa, transition.target(), depth,-1 , perft_depth); 
        if(rec_result)
        {
          calculation_state_->SetPos(previous_pos);
          return true;;
        }
      }
      calculation_state_->SetPos(previous_pos);
      continue;
    }
    bool found = false;
    ActionResult result = transition.letter()->Apply(calculation_state_);
    if (result) {
      if (transition.letter()->IsModifier()) {
        size_t new_depth = depth;
        if (last_block_started != transition.letter()->index()) {
          new_depth = depth + 1;
          CreateVisitedLayers(visited_array_index, depth + 1);
        }
        if (transition.letter()->IsSwitch()) {
          found = true;
          if (calculation_state_->player() == calculation_state_->description().keeper_player_id()) {
            if (!FastPerftKeeper(visited_array_index, nfa, transition.target(), new_depth, transition.letter()->index(), perft_depth)) {
              node_count_++;
              if (perft_depth == 1) leaf_count_++;
            }
          }
          else {
          node_count_++;
          if (perft_depth > 1) {
            FastPerft(visited_array_index, nfa, transition.target(), new_depth, transition.letter()->index(), perft_depth - 1);
          }
          else {
            leaf_count_ += 1;
          }
          }
          
        }
        else {
          auto rec_result = FastPerftKeeper(visited_array_index, nfa, transition.target(), new_depth, transition.letter()->index(), perft_depth); 
          found = rec_result;
        }
      } else {
        auto rec_result = FastPerftKeeper(visited_array_index, nfa, transition.target(), depth, -1, perft_depth); 
        found = rec_result;
      }
    }
    transition.letter()->Revert(calculation_state_, result);
    //if(calculation_state_->player() == calculation_state_->description().keeper_player_id() && (node_count_ > 0))
    if(found)
    {
      return true;
    }
  }
  return false;
}



PerftResult SearchContext::FindMovesDeep(GameState *state, size_t perft_depth, ssize_t maximal_depth) {
  calculation_state_ = state;
  leaf_count_ = 0;
  node_count_ = 0;
  if (maximal_depth >= 0)
    max_depth_ = (size_t) maximal_depth;
  size_t visited_index = NewVisited(
      state->description().moves_description().nfa());
  Move empty;
  if (calculation_state_->player() ==  calculation_state_->description().keeper_player_id()) {
      FastPerftKeeper(visited_index, state->description().moves_description().nfa(),
                       state->nfa_state(), 0, false, perft_depth);
      DumpVisited(visited_index);
      calculation_state_ = nullptr;
      return {leaf_count_, node_count_};
  } else {
      FastPerft(visited_index, state->description().moves_description().nfa(),
                       state->nfa_state(), 0, false, perft_depth);
      DumpVisited(visited_index);
      calculation_state_ = nullptr;
      return {leaf_count_, node_count_+1};
  }
}

