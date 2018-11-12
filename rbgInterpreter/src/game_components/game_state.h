//
// Created by ??? on 12.01.18.
//

#ifndef RBGGAMEMANAGER_GAME_STATE_H
#define RBGGAMEMANAGER_GAME_STATE_H


#include <utility>

#include "game_description.h"
#include "search_context.h"

// This MoveResult can be used to revert moves and get
// information about move.
class MoveResult {
public:
  MoveResult(std::vector<ActionResult> actions_results, vertex_t previous_pos,
             fsm::state_id_t previous_state,
             std::vector<ActionApplication> applied_modifiers)
      : actions_results_(std::move(actions_results)),
        previous_pos_(previous_pos),
        previous_state_(previous_state),
        applied_modifiers_(std::move(applied_modifiers)) {}

  const std::vector<ActionResult> &actions_results() const {
    return actions_results_;
  }

  const std::vector<ActionApplication> &applied_modifiers() const {
    return applied_modifiers_;
  }

  vertex_t previous_pos() const
  {
    return previous_pos_;
  }

  fsm::state_id_t previous_state() const {
    return previous_state_;
  }

private:
  std::vector<ActionResult> actions_results_;
  vertex_t previous_pos_;
  fsm::state_id_t previous_state_;
  std::vector<ActionApplication> applied_modifiers_;
};

class GameState {
public:
  // Lazy controller is separated from main class
  friend class LazyEvaluator;

  // Modifying actions

  friend class actions::Shift;

  friend class actions::Off;

  friend class actions::PlayerSwitch;

  friend class actions::Assignment;

  friend class actions::ConditionCheck;

  friend class actions::NegatedConditionCheck;

  // Move pattern will be able to use the state_ to check the condition
  // This will speed up things. (We do not have to make a copy.)

  explicit GameState(const GameDescription &description)
      : parent_(description),
        current_board_(description.initial_board()),
        current_pos_(0),
        current_state_(description.moves_description().nfa().initial()),
        sigma_(description.VariablesCount(), 0),
        current_player_(description.keeper_player_id()) {
    for (vertex_t v= 0; v < static_cast<ssize_t >(current_board_.size()); v++) {
        sigma_[current_board_[v]]++;
    }
  }

  // Resets the game to the initial state_.
  void Reset() {
    current_board_ = parent_.initial_board();
    current_pos_ = 0;
    current_state_ = parent_.moves_description().nfa().initial();
    std::fill(sigma_.begin(), sigma_.end(), 0);
    for (vertex_t v= 0; v < static_cast<ssize_t >(current_board_.size()); v++) {
      sigma_[current_board_[v]]++;
    }
    current_player_ = parent_.keeper_player_id();
  }

  fsm::state_id_t nfa_state() const {
    return current_state_;
  }

  // Returns the piece at current position.
  token_id_t CurrentPiece() const {
    return current_board_[current_pos_];
  }

  const GameDescription &description() const {
    return parent_;
  }

  int Value(token_id_t variable) const {
    return sigma_[variable];
  }

  const GraphBoard &board() const {
    return current_board_;
  }

  vertex_t pos() const
  {
    return current_pos_;
  }

  void SetPos(vertex_t new_pos)
  {
    current_pos_ = new_pos;
  }

  token_id_t player() const {
    return current_player_;
  }

  void MakeMove(const Move &move) {
    for (const auto &block : move.blocks()) {
      current_state_ = parent_.moves_description().CorrespondingState(
          block.id());
      current_pos_ = block.pos();
      while (
          parent_.moves_description().nfa()[current_state_].transitions().size() ==
          1 &&
          parent_.moves_description().nfa()[current_state_].transitions().front().letter()->index() ==
          block.id()) {
        parent_.moves_description().nfa()[current_state_].transitions().front().letter()->Apply(
            this);
        current_state_ = parent_.moves_description().nfa()[current_state_].transitions().front().target();
      }
    }
  }

  MoveResult MakeRevertibleMove(const Move &move) {
    std::vector<ActionResult> results;
    std::vector<ActionApplication> applied_modifiers;
    vertex_t previous_pos = current_pos_;
    fsm::state_id_t previous_state = current_state_;
    for (const auto &block : move.blocks()) {
      current_state_ = parent_.moves_description().CorrespondingState(
          block.id());
      current_pos_ = block.pos();
      while (
          parent_.moves_description().nfa()[current_state_].transitions().size() ==
          1 &&
          parent_.moves_description().nfa()[current_state_].transitions().front().letter()->index() ==
          block.id()) {
        results.push_back(
            parent_.moves_description().nfa()[current_state_].transitions().front().letter()->Apply(
                this));
        applied_modifiers.emplace_back(current_pos_, parent_.moves_description().nfa()[current_state_].transitions().front().letter());
        current_state_ = parent_.moves_description().nfa()[current_state_].transitions().front().target();
      }
    }
    return {std::move(results), previous_pos, previous_state,
            std::move(applied_modifiers)};
  }

  void RevertMove(const MoveResult &information) {
    for (ssize_t i = information.applied_modifiers().size() - 1; i >= 0; i--) {
      const auto &action = information.applied_modifiers()[i];
      RevertActionApplication(action, information.actions_results()[i]);
    }
    current_pos_ = information.previous_pos();
    current_state_ = information.previous_state();
  }

  std::vector<Move> FindMoves(SearchContext *context) {
    current_search_ = context;
    auto result = current_search_->FindMoves(this);
    current_search_ = nullptr;
    return std::move(result);
  }

  void FindMoves(SearchContext *context, std::vector<Move> *moves) {
    current_search_ = context;
    current_search_->FindMoves(this, moves);
    current_search_ = nullptr;
  }
  
  PerftResult FindMovesDeep(SearchContext *context, size_t perft_depth, ssize_t max_depth=-1) {
    current_search_ = context;
    auto result = current_search_->FindMovesDeep(this, perft_depth, max_depth);
    current_search_ = nullptr;
    return std::move(result);
  }

  std::vector<Move>
  FindFirstMove(SearchContext *context) {
    current_search_ = context;
    auto result = current_search_->FindFirstMove(this);
    current_search_ = nullptr;
    return std::move(result);
  }

  void FindFirstMove(SearchContext *context, std::vector<Move> *moves) {
    current_search_ = context;
    current_search_->FindFirstMove(this, moves);
    current_search_ = nullptr;
  }
  
  bool ApplyFirstMove(SearchContext *context) {
    current_search_ = context;
    bool result = current_search_->ApplyFirstMove(this);
    current_search_ = nullptr;
    return result;
  }

  bool ApplyFirstRandomMove(SearchContext *context) {
    current_search_ = context;
    bool result = current_search_->ApplyFirstRandomMove(this);
    current_search_ = nullptr;
    return result;
  }
  
public:  
  const GameDescription &parent_;
private:
  GraphBoard current_board_;
  vertex_t current_pos_;
public:
  fsm::state_id_t current_state_;
private:
  std::vector<int> sigma_;
  token_id_t current_player_;

  SearchContext *current_search_;

  void SetPiece(token_id_t piece) {
    sigma_[CurrentPiece()]--;
    current_board_[current_pos_] = piece;
    sigma_[piece]++;
  }

  bool SearchingForMoves() {
    return current_search_ != nullptr;
  }

  SearchContext &search_context() {
    return *current_search_;
  }

  ActionResult ApplyActionApplication(const ActionApplication &application);

  void RevertActionApplication(const ActionApplication &application,
                               const ActionResult &application_result);
};

std::ostream &operator<<(std::ostream &s, const GameState &state);

#endif //RBGGAMEMANAGER_GAME_STATE_H
