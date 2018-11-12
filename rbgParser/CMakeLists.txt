
cmake_minimum_required(VERSION 3.5.0)
project(rbgParser)

set(CMAKE_CXXFLAGS "${CMAKE_CXX_FLAGS} ")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ")

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES src/switch.hpp
src/identifier_leaf.hpp
src/identifier_leaf.cpp
src/internal_node.hpp
src/assignments.cpp
src/ons.cpp
src/rules_parser.hpp
src/shift.hpp
src/integer_leaf.hpp
src/tree_parser.cpp
src/arithmetic_operation.hpp
src/tree_parser.hpp
src/arithmetic_expression.hpp
src/shift.cpp
src/graph_builder.hpp
src/macro_bank.hpp
src/offs.cpp
src/sum.cpp
src/move_check.cpp
src/hexagon2D.hpp
src/tree_utils.hpp
src/cuboid3D.cpp
src/macro_bank.cpp
src/token.cpp
src/printer_helpers.cpp
src/printer_helpers.hpp
src/arithmetic_comparison.cpp
src/offs.hpp
src/concatenation.cpp
src/types.hpp
src/message.cpp
src/graph.cpp
src/variable_arithmetic.hpp
src/parser_helpers.cpp
src/concatenation.hpp
src/game_items.hpp
src/star_move.cpp
src/parser_helpers.hpp
src/graph.hpp
src/typing_machine.hpp
src/slice.cpp
src/hexagon2D.cpp
src/slice.hpp
src/tree_utils.cpp
src/options.cpp
src/arithmetic_operation.cpp
src/message.hpp
src/declarations.hpp
src/arithmetic_comparison.hpp
src/sum.hpp
src/typing_machine.cpp
src/rules_parser.cpp
src/straightness_helpers.hpp
src/abstract_dispatcher.hpp
src/options.hpp
src/declarations.cpp
src/star_move.hpp
src/bracketed_expression.cpp
src/arrow_leaf.hpp
src/parsed_game.cpp
src/ons.hpp
src/assignments.hpp
src/internal_node.cpp
src/unchecked_graph.cpp
src/token.hpp
src/parsed_game.hpp
src/integer_arithmetic.hpp
src/integer_leaf.cpp
src/slice_iterator.hpp
src/move_check.hpp
src/bracketed_expression.hpp
src/integer_arithmetic.cpp
src/arrow_leaf.cpp
src/switch.cpp
src/cuboid3D.hpp
src/straightness_helpers.cpp
src/variable_arithmetic.cpp
src/rectangle2D.cpp
src/slice_iterator.cpp
src/game_move.hpp
src/rectangle2D.hpp
src/game_items.cpp
src/unchecked_graph.hpp)
add_library(rbgParser ${SOURCE_FILES})

