//
// Created by ??? on 07.05.18.
//

#ifndef RBGGAMEMANAGER_ARITHMETIC_CREATOR_H
#define RBGGAMEMANAGER_ARITHMETIC_CREATOR_H

#include<sum.hpp>
#include<concatenation.hpp>
#include<star_move.hpp>
#include<shift.hpp>
#include<ons.hpp>
#include<offs.hpp>
#include<assignments.hpp>
#include<switch.hpp>
#include<arithmetic_comparison.hpp>
#include<integer_arithmetic.hpp>
#include<variable_arithmetic.hpp>
#include<arithmetic_operation.hpp>
#include "../game_components/name_resolver.h"
#include "../actions/arithmetic_operation.h"

std::unique_ptr<ArithmeticOperation> CreateOperation(const rbg_parser::arithmetic_expression& expression, const NameResolver& resolver);




#endif //RBGGAMEMANAGER_ARITHMETIC_CREATOR_H
