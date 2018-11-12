//
// Created by ??? on 12.01.18.
//

#ifndef RBGGAMEMANAGER_ACTION_H
#define RBGGAMEMANAGER_ACTION_H

#include <unordered_set>
#include <memory>
#include <iostream>

#include "../game_components/name_resolver.h"
#include "action_types.h"
#include "action_application.h"
#include "../game_nfa/automaton.h"
#include "arithmetic_operation.h"
#include "../game_components/graph_board.h"

class GameState;

// Represents abstract action that can be called on a game state.
// Actions can be applied and reverted.
// Example:
//   Action& a = actions::Shift(2,2);
//   auto result = a.Apply(state);
class Action {
public:
  virtual ActionResult Apply(GameState *b) const =0;

  // Revert should be applied even if the action was not successful.
  virtual void Revert(GameState *, const ActionResult &) const {}

  bool IsModifier() const { return IsModifierType(type_); }

  bool IsSwitch() const { return IsSwitchType(type_); }

  ActionType type() const { return type_; }

  // Index is meaningful iff IsModifier() == true.
  unsigned int index() const { return index_; }

  virtual ~Action() = default;

protected:
  Action(ActionType type, unsigned int action_index)
      : type_(type), index_(action_index) {}

  explicit Action(ActionType type) : type_(type), index_(0) {}

private:
  ActionType type_;
  uint index_;
};

namespace actions {
  // Empty action does not change the game state.
  class Empty : public Action {
  public:
    Empty() : Action(ActionType::kEmptyType) {}

    ActionResult Apply(GameState *) const override { return true; };
  };

  // Shift changes the current position on board.
  class Shift : public Action {
  public:
    Shift(edge_name_t edge_name) : Action(ActionType::kShiftType), edge_name_(edge_name)
                                {}

    ActionResult Apply(GameState *b) const override;

    void Revert(GameState *state, const ActionResult &apply_result) const override;

    // Returns the edge this shift travels.
    edge_name_t edge_name() const { return edge_name_; }

  private:
    edge_name_t edge_name_;
  };

  // Shift changes the current position on board.
  class ShiftTable : public Action {
  public:
    explicit ShiftTable(std::vector<std::vector<vertex_t> > table) : Action(ActionType::kShiftTableType), table_(std::move(table))
    {
        for (uint i = 0; i < table_.size(); i++) {
          std::vector<vertex_t> new_row;
          new_row.reserve(table_[i].size());
          for (uint j = 0; j < table_[i].size(); j++) {
            if (table_[i][j] != -1) new_row.push_back(table_[i][j]);
          }
          table_[i] = std::move(new_row);
          table_[i].shrink_to_fit();
        }
    }

    ActionResult Apply(GameState *) const override {
      std::cerr << "ShiftTable action should not be applied." << std::endl;
      return false;
    }
    void Revert(GameState *, const ActionResult &) const override {};

    // Returns the edge this shift travels.
    const std::vector<std::vector<vertex_t> >& table() const { return table_; }

  private:
    std::vector<std::vector<vertex_t> > table_;
  };

  // On checks if one of the pieces is on the current position on board.
  class On : public Action {
  public:
    // Parameter pieces should contain true for token ids of pieces names that the on accepts.
    explicit On(std::vector<bool> pieces)
        : Action(ActionType::kOnType), pieces_(std::move(pieces)) {}

    ActionResult Apply(GameState *b) const override;

    // Returns the pieces on requires on the board in order to be legal.
    const std::vector<bool> &pieces() const { return pieces_; }

  private:
    std::vector<bool> pieces_;
  };

  // Check arithmetic comparison
  class ArithmeticLessComparison : public Action {
  public:
    ArithmeticLessComparison(std::unique_ptr<ArithmeticOperation> left,
                            std::unique_ptr<ArithmeticOperation> right)
        : Action(ActionType ::kArithmeticLessComparisonType),
          left_(std::move(left)),
          right_(std::move(right))
    {}

    ActionResult Apply(GameState *b) const override;

    const ArithmeticOperation* left() const
    {
      return left_.get();
    }

    const ArithmeticOperation* right() const
    {
      return right_.get();
    }

  private:
    std::unique_ptr<ArithmeticOperation> left_, right_;
  };

  // Check arithmetic comparison
  class ArithmeticLessEqualComparison : public Action {
  public:
    ArithmeticLessEqualComparison(std::unique_ptr<ArithmeticOperation> left,
                             std::unique_ptr<ArithmeticOperation> right)
        : Action(ActionType ::kArithmeticLessEqualComparisonType),
          left_(std::move(left)),
          right_(std::move(right))
    {}

    ActionResult Apply(GameState *b) const override;

    const ArithmeticOperation* left() const
    {
      return left_.get();
    }

    const ArithmeticOperation* right() const
    {
      return right_.get();
    }

  private:
    std::unique_ptr<ArithmeticOperation> left_, right_;
  };

  // Check arithmetic comparison
  class ArithmeticEqualComparison : public Action {
  public:
    ArithmeticEqualComparison(std::unique_ptr<ArithmeticOperation> left,
                             std::unique_ptr<ArithmeticOperation> right)
        : Action(ActionType ::kArithmeticEqualComparisonType),
          left_(std::move(left)),
          right_(std::move(right))
    {}

    ActionResult Apply(GameState *b) const override;

    const ArithmeticOperation* left() const
    {
      return left_.get();
    }

    const ArithmeticOperation* right() const
    {
      return right_.get();
    }

  private:
    std::unique_ptr<ArithmeticOperation> left_, right_;
  };

  // Check arithmetic comparison
  class ArithmeticNotEqualComparison : public Action {
  public:
    ArithmeticNotEqualComparison(std::unique_ptr<ArithmeticOperation> left,
                              std::unique_ptr<ArithmeticOperation> right)
        : Action(ActionType ::kArithmeticNotEqualComparisonType),
          left_(std::move(left)),
          right_(std::move(right))
    {}

    ActionResult Apply(GameState *b) const override;

    const ArithmeticOperation* left() const
    {
      return left_.get();
    }

    const ArithmeticOperation* right() const
    {
      return right_.get();
    }

  private:
    std::unique_ptr<ArithmeticOperation> left_, right_;
  };

  // Check move pattern
  class ConditionCheck : public Action {
  public:
    ConditionCheck(unsigned int index,
                   std::unique_ptr<fsm::Nfa<const Action *> > move_nfa)
        : Action(ActionType ::kConditionCheckType),
          index_(index),
          move_nfa_(std::move(move_nfa))
    {}

    ActionResult Apply(GameState *b) const override;

    const fsm::Nfa<const Action *> *nfa() const {
      return move_nfa_.get();
    }
  private:
    unsigned int index_;
    std::unique_ptr<fsm::Nfa<const Action *> > move_nfa_;
  };

  // Check negated move pattern
  class NegatedConditionCheck : public Action {
  public:
    NegatedConditionCheck(unsigned int index,
                   std::unique_ptr<fsm::Nfa<const Action *> > move_nfa)
        : Action(ActionType ::kNegatedConditionCheckType),
          index_(index),
          move_nfa_(std::move(move_nfa))
    {}

    ActionResult Apply(GameState *b) const override;

    const fsm::Nfa<const Action *> *nfa() const {
      return move_nfa_.get();
    }
  private:
    unsigned int index_;
    std::unique_ptr<fsm::Nfa<const Action *> > move_nfa_;
  };

  // Off places specified piece on the current position in game state.
  class Off : public Action {
  public:
    explicit Off(token_id_t piece, unsigned int index = 0) : Action(
        ActionType::kOffType, index), piece_(piece) {}

    ActionResult Apply(GameState *b) const override;

    void Revert(GameState *b, const ActionResult &apply_result) const override;

    token_id_t piece() const { return piece_; }

  private:
    token_id_t piece_;
  };

  // PlayerSwitch changes the player to the one specified.
  class PlayerSwitch : public Action {
  public:
    explicit PlayerSwitch(token_id_t player, unsigned int index = 0) : Action(
        ActionType::kSwitchType, index), player_(player) {}

    ActionResult Apply(GameState *b) const override;

    void Revert(GameState *b, const ActionResult &apply_result) const override;

    token_id_t player() const { return player_; }

  private:
    token_id_t player_;
  };

  // Assignment assigns a value to the variable.
  class Assignment : public Action {
  public:
    Assignment(size_t variable, std::unique_ptr<ArithmeticOperation> value,
               unsigned int index = 0)
        : Action(ActionType::kAssignmentType, index),
          variable(variable),
          value(std::move(value)) {}

    ActionResult Apply(GameState *b) const override;

    void Revert(GameState *b, const ActionResult &apply_result) const override;

    token_id_t get_variable() const { return variable; }

    const ArithmeticOperation *get_value_expression() const {
      return value.get();
    }

  private:
    token_id_t variable;
    std::unique_ptr<ArithmeticOperation> value;
  };

  class PlayerCheck : public Action {
  public:
    explicit PlayerCheck(token_id_t player)
        : Action(ActionType::kPlayerCheck), player_(player)
    {
    }

    ActionResult Apply(GameState *b) const override;

    token_id_t player() const
    {
      return player_;
    }

  private:
    token_id_t player_;
  };
}


#endif //RBGGAMEMANAGER_ACTION_H
