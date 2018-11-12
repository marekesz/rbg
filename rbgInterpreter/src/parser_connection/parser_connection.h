//
// Created by ??? on 13.01.18.
//

#ifndef RBGGAMEMANAGER_PARSER_CONNECTION_H
#define RBGGAMEMANAGER_PARSER_CONNECTION_H

#include <graph.hpp>
#include <declarations.hpp>
#include <game_move.hpp>
#include <parsed_game.hpp>

class Board;

class NameResolver;

class GameMovesDescription;

class GameDescription;

class Declarations;

class GraphBoard;

class EdgeResolver;

NameResolver CreateResolver(const rbg_parser::declarations &declarations);

Board CreateBoard(const rbg_parser::graph &parsed_board,
                  const NameResolver &resolver);

EdgeResolver CreateEdgeNamesResolver(const rbg_parser::graph &parsed_graph_board);

GraphBoard CreateGraphBoard(const rbg_parser::graph &parsed_graph_board);

Declarations CreateDeclarations(const rbg_parser::declarations &declarations,
                                const NameResolver &resolver);

GameMovesDescription
CreateMoves(const rbg_parser::game_move &move, const NameResolver &resolver);

GameDescription CreateDescription(const rbg_parser::parsed_game &game);

#endif //RBGGAMEMANAGER_PARSER_CONNECTION_H
