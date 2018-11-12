//
// Created by ??? on 12.01.18.
//

#ifndef RBGGAMEMANAGER_ARITHMETIC_OPERATION_H
#define RBGGAMEMANAGER_ARITHMETIC_OPERATION_H

#include <bits/unique_ptr.h>
#include "../game_components/name_resolver.h"
#include "action_types.h"

class GameState;

// Used to represent all possible arithmetic operations that
// can be created from states variables and constant values.
// Example:
//   ArithmeticOperation& ao = arithmetic_operations::variable(17);
//   int value = ao.Value(state);
//   if (value == state.Value(17))
class ArithmeticOperation {
public:
  virtual int Value(GameState *b) const =0;

  virtual ~ArithmeticOperation() = default;

  ArithmeticOperationType type() const { return type_; }

protected:
  explicit ArithmeticOperation(ArithmeticOperationType type) : type_(type) {}

private:
  ArithmeticOperationType type_;
};

namespace arithmetic_operations {
  // Gives the value of the variable in current state.
  class Variable : public ArithmeticOperation {
  public:
    explicit Variable(token_id_t variable) :
        ArithmeticOperation(ArithmeticOperationType::kVariableType),
        variable_(variable) {}

    int Value(GameState *b) const override;

    token_id_t variable() const {
      return variable_;
    }

  private:
    token_id_t variable_;
  };

  class Constant : public ArithmeticOperation {
  public:
    explicit Constant(int constant) :
        ArithmeticOperation(ArithmeticOperationType::kConstantType),
        constant_(constant) {}

    int Value(GameState *b) const override;

    int Value() const { return Value(nullptr); };

    int constant() {
      return constant_;
    }

  private:
    int constant_;
  };

  class Sum : public ArithmeticOperation {
  public:
    explicit Sum(std::vector<std::unique_ptr<ArithmeticOperation> > operations) :
        ArithmeticOperation(ArithmeticOperationType::kSumType),
        operations_(std::move(operations))
    {}

    int Value(GameState *b) const override;

    const std::vector<std::unique_ptr<ArithmeticOperation> >& operations() const
    {
      return operations_;
    }
  private:
    std::vector<std::unique_ptr<ArithmeticOperation> > operations_;
  };

  class Subtraction : public ArithmeticOperation {
  public:
    explicit Subtraction(std::vector<std::unique_ptr<ArithmeticOperation> > operations) :
    ArithmeticOperation(ArithmeticOperationType::kSubtractionType),
    operations_(std::move(operations))
    {}

    int Value(GameState *b) const override;

    const std::vector<std::unique_ptr<ArithmeticOperation> >& operations() const
    {
      return operations_;
    }
  private:
    std::vector<std::unique_ptr<ArithmeticOperation> > operations_;
  };

  class Product : public ArithmeticOperation {
  public:
    explicit Product(std::vector<std::unique_ptr<ArithmeticOperation> > operations) :
    ArithmeticOperation(ArithmeticOperationType::kProductType),
    operations_(std::move(operations))
    {}

    int Value(GameState *b) const override;

    const std::vector<std::unique_ptr<ArithmeticOperation> >& operations() const
    {
      return operations_;
    }
  private:
    std::vector<std::unique_ptr<ArithmeticOperation>> operations_;
  };

  class Division : public ArithmeticOperation {
  public:
    explicit Division(std::vector<std::unique_ptr<ArithmeticOperation> > operations) :
        ArithmeticOperation(ArithmeticOperationType::kDivisionType),
        operations_(std::move(operations))
    {}

    int Value(GameState *b) const override;

    const std::vector<std::unique_ptr<ArithmeticOperation> >& operations() const
    {
      return operations_;
    }
  private:
    std::vector<std::unique_ptr<ArithmeticOperation> > operations_;
  };
}


#endif //RBGGAMEMANAGER_ARITHMETIC_OPERATION_H
