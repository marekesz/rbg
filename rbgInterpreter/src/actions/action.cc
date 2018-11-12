//
// Created by ??? on 12.01.18.
//

#include "action.h"
#include "../game_components/game_state.h"

ActionResult actions::Shift::Apply(GameState *b) const {
  vertex_t previous_pos = b->current_pos_;
  b->current_pos_ = b->current_board_.Next(b->current_pos_,edge_name_);
  return {b->current_pos_ >= 0, previous_pos};
}

void actions::Shift::Revert(GameState *state, const ActionResult & r) const {
  state->current_pos_ = r.revert_pos();
}

ActionResult actions::On::Apply(GameState *b) const {
  return {pieces_[b->CurrentPiece()]};
}

ActionResult actions::Off::Apply(GameState *b) const {
  size_t previous_piece = b->CurrentPiece();
  b->SetPiece(piece_);
  return {true, previous_piece};
}

void
actions::Off::Revert(GameState *b, const ActionResult &apply_result) const {
  b->SetPiece(apply_result.revert_piece());
}

ActionResult actions::PlayerSwitch::Apply(GameState *b) const {
  size_t previous_player = b->player();
  b->current_player_ = player_;
  return {true, previous_player};
}

void actions::PlayerSwitch::Revert(GameState *b,
                                   const ActionResult &apply_result) const {
  b->current_player_ = apply_result.revert_player();
}

ActionResult actions::Assignment::Apply(GameState *b) const {
  int previous_value = b->Value(variable);
  b->sigma_[variable] = value->Value(b);
  return {0 <= b->sigma_[variable] && b->sigma_[variable] <= b->description().declarations().bound(variable), previous_value};
}

void actions::Assignment::Revert(GameState *b,
                                 const ActionResult &apply_result) const {
  b->sigma_[variable] = apply_result.revert_value();
}

ActionResult actions::ConditionCheck::Apply(GameState *b) const {
  return b->search_context().CheckPattern(*move_nfa_);
}

ActionResult actions::NegatedConditionCheck::Apply(GameState *b) const {
  return !b->search_context().CheckPattern(*move_nfa_);
}

ActionResult actions::ArithmeticLessComparison::Apply(GameState *b) const {
  return left_->Value(b) < right_->Value(b);
}

ActionResult actions::ArithmeticEqualComparison::Apply(GameState *b) const {
  return left_->Value(b) == right_->Value(b);
}

ActionResult actions::ArithmeticLessEqualComparison::Apply(GameState *b) const {
  return left_->Value(b) <= right_->Value(b);
}

ActionResult actions::ArithmeticNotEqualComparison::Apply(GameState *b) const {
  return left_->Value(b) != right_->Value(b);
}

ActionResult actions::PlayerCheck::Apply(GameState *b) const {
  return b->player() == player_;
}
