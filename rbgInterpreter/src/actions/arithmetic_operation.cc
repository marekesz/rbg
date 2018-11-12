//
// Created by ??? on 12.01.18.
//

#include "arithmetic_operation.h"
#include "../game_components/game_state.h"

int arithmetic_operations::Variable::Value(GameState *b) const {
  return b->Value(variable_);
}

int arithmetic_operations::Constant::Value(GameState *) const {
  return constant_;
}

int arithmetic_operations::Sum::Value(GameState *b) const {
  int result = 0;
  for(const auto& child : operations_)
  {
    result += child->Value(b);
  }
  return result;
}

int arithmetic_operations::Product::Value(GameState *b) const {
  int result = 1;
  for(const auto& child : operations_)
  {
    result *= child->Value(b);
  }
  return result;
}

int arithmetic_operations::Subtraction::Value(GameState *b) const {
  // TODO(???): Ask about the semantics of this operation.
  int result = operations_[0]->Value(b);
  for(size_t i = 1; i < operations_.size(); i++)
  {
    const auto& child = operations_[i];
    result -= child->Value(b);
  }
  return result;
}

int arithmetic_operations::Division::Value(GameState *b) const {
  // TODO(???): Ask about the semantics of this operation.
  int result = operations_[0]->Value(b);
  for(size_t i = 1; i < operations_.size(); i++)
  {
    const auto& child = operations_[i];
    result /= child->Value(b);
  }
  return result;
}
