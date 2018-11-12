//
// Created by ??? on 01/02/18.
//

#include <boost/asio/io_service.hpp>

#include <iostream>
#include <fstream>
#include <sstream>

#include <memory>
#include <string>
#include <vector>

#include <message.hpp>
#include <parsed_game.hpp>
#include <game_items.hpp>
#include "game_components/game_description.h"
#include "networking/server.h"

int main(int argc, char *argv[]) {
  /* TODO(???): Make proper program options with boost. */
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <filename> <port>\n";
    return 1;
  }

  rbg_parser::messages_container msg;
  std::string filename(argv[argc - 2]);
  std::ifstream t(filename);
  std::stringstream buffer;
  buffer << t.rdbuf();

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

  /* TODO(???): Do it once in the future.
  There were some differences with to_rbg game and original. */
  std::stringstream new_buffer(pg->to_rbg(true));
  msg = {};

  try {
    std::vector<rbg_parser::token> result = tokenize(new_buffer.str(), msg);
    rbg_parser::game_items g = input_tokens(result, msg);
    pg = std::unique_ptr<rbg_parser::parsed_game>(
        new rbg_parser::parsed_game(g.parse_game(msg)));
  } catch (rbg_parser::message &m) {
    std::cout << "Game description is bad. Here is the error: " << "\n";
    std::cout << m.as_error() << std::endl;
    return 1;
  }

  GameDescription description = CreateDescription(*pg);

  try {
    boost::asio::io_service io_service;
    Server server(description, io_service,
                  static_cast<unsigned short>(std::atoi(argv[argc - 1])));

    io_service.run();
  }
  catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
