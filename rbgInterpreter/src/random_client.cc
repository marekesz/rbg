//
// Created by ??? on 01/02/18.
//

#include <iostream>
#include <memory>
#include <vector>

#include "networking/client.h"
#include <game_items.hpp>
#include "game_components/game_description.h"
#include "parser_connection/parser_connection.h"
#include "game_components/game_state.h"

int main(int argc, char *argv[]) {
  /* TODO(???): Make proper program options with boost. */
  if (argc != 3) {
    std::cerr << "Usage " << argv[0] << " <host> <port>\n";
    return 1;
  }

  try {
    srand(static_cast<unsigned int>(time(nullptr)));

    rbg_parser::messages_container msg;
    SynchronousClient client(argv[argc - 2], argv[argc - 1]);
    std::stringstream buffer(client.description());

    std::unique_ptr<rbg_parser::parsed_game> pg;
    try {
      std::vector<rbg_parser::token> result = tokenize(buffer.str(), msg);
      rbg_parser::game_items g = input_tokens(result, msg);
      pg = std::unique_ptr<rbg_parser::parsed_game>(
          new rbg_parser::parsed_game(g.parse_game(msg)));
    } catch (rbg_parser::message &m) {
      std::cout << "Game description is bad. Here is the error: " << "\n";
      std::cout << m.as_error() << std::endl;
      return 1;
    }
    GameDescription gd(CreateDescription(*pg));

    SearchContext context;
    GameState state(gd);

    token_id_t assigned_player = gd.resolver().Id(client.player());

    std::cout << "I am player: " << gd.resolver().Name(assigned_player)
              << std::endl;

    auto moves = state.FindMoves(&context);
    while (!moves.empty()) {
      if (state.player() == assigned_player) {
        auto move = moves[rand() % moves.size()];
        state.MakeMove(move);
        client.Write(move);
        moves = state.FindMoves(&context);
      } else {
        auto move = client.Read();
        state.MakeMove(move);
        moves = state.FindMoves(&context);
      }
    }
  }
  catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
