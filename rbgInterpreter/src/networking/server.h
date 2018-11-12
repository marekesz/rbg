//
// Created by ??? on 31/01/18.
//

#ifndef RBGGAMEMANAGER_SERVER_H
#define RBGGAMEMANAGER_SERVER_H

#include <boost/asio.hpp>

#include <deque>

#include "message.h"
#include "../game_components/game_description.h"
#include "../game_components/game_state.h"
#include "../../rbgParser/src/game_items.hpp"
#include "../parser_connection/parser_connection.h"

using boost::asio::ip::tcp;

class ServerGameInstance;

class ClientConnection
    : public std::enable_shared_from_this<ClientConnection> {
public:
  typedef std::shared_ptr<ClientConnection> pointer;

  static pointer Create(tcp::socket socket, ServerGameInstance *instance) {
    return pointer(new ClientConnection(std::move(socket), instance));
  }

  tcp::socket &socket() {
    return socket_;
  }

  void Start();

  void Send(const Message &message) {
    bool write_in_progress = !write_moves_.empty();
    write_moves_.push_back(message);
    if (!write_in_progress) {
      Write();
    }
  }

private:
  tcp::socket socket_;

  Message read_move_;
  std::deque<Message> write_moves_;

  ServerGameInstance *instance_;

  ClientConnection(tcp::socket socket, ServerGameInstance *instance) :
      socket_(std::move(socket)), read_move_(Message::Empty()),
      instance_(instance) {}

  void ReadHeader();

  void ReadBody();

  void Write();
};

class Server;

class ServerGameInstance {
public:
  explicit ServerGameInstance(const GameDescription &description,
                              Server *controlling_server = nullptr)
      : state_(description), controlling_server_(controlling_server) {
    for (token_id_t player_token : description.declarations().players_ids()) {
      players_[player_token] = nullptr;
    }
  }

  void Join(const ClientConnection::pointer &client) {
    std::cout << client << " joins the game." << std::endl;
    for (token_id_t player_token :
        state_.description().declarations().players_ids()) {
      if (!players_[player_token]) {
        clients_[client] = player_token;
        players_[player_token] = client;
        std::cout << client << " is player "
                  << state_.description().resolver().Name(player_token)
                  << "." << std::endl;
        if (Full())
          Start();
        return;
      }
    }
    std::cout << client << " has no player assigned." << std::endl;
  }

  void Start() {
    std::cout << "Starting the game." << std::endl;
    for (const auto &pair : clients_) {
      const auto &client = pair.first;
      client->Send(state_.description().text_description());
      client->Send(state_.description().resolver().Name(pair.second));
    }
    MakeKeeperMoves();
    auto moves = state_.FindMoves(&context_);
    available_moves_ = std::unordered_set<Move>(moves.begin(), moves.end());
    std::cout << state_ << std::endl;
    if (available_moves_.empty()) {
      Stop();
    }
  }

  void Stop() {
    std::cout << "Game ended." << std::endl;
    for (const auto &pair : clients_) {
      std::cout << "\t" << state_.description().resolver().Name(pair.second)
                << "(" << pair.first << ")" << " : "
                << state_.Value(pair.second) << std::endl;
    }
  }

  bool Full() const {
    return clients_.size() ==
           state_.description().declarations().players_ids().size();
  }

  void Leave(const ClientConnection::pointer &client);

  void Deliver(const Message &msg) {
    for (const auto &pair : clients_) {
      pair.first->Send(msg);
    }
  }

  void Deliver(const Message &msg, const ClientConnection::pointer &omit) {
    for (const auto &pair : clients_) {
      if (pair.first && pair.first != omit)
        pair.first->Send(msg);
    }
  }

  bool
  HandleMove(const ClientConnection::pointer &client, const Message &message) {
    token_id_t client_player_id = clients_[client];
    std::cout << "Got a move from client " << client
              << " which is player "
              << state_.description().resolver().Name(client_player_id)
              << std::endl;
    if (client_player_id != state_.player()) {
      std::cout << "Wrong player." << std::endl;
      return false;
    }
    auto move = DecodeMove(message);
    if (available_moves_.find(move) == available_moves_.end()) {
      std::cout << "Wrong move." << std::endl;
      return false;
    }
    state_.MakeMove(move);
    Deliver(message, client);
    MakeKeeperMoves();
    auto moves = state_.FindMoves(&context_);
    available_moves_ = std::unordered_set<Move>(moves.begin(), moves.end());
    std::cout << state_ << std::endl;
    if (available_moves_.empty()) {
      Stop();
    }
    return true;
  }

private:
  std::unordered_map<ClientConnection::pointer, token_id_t> clients_;
  std::unordered_map<token_id_t, ClientConnection::pointer> players_;
  GameState state_;
  Server *controlling_server_;
  std::unordered_set<Move> available_moves_;
  SearchContext context_;

  void MakeKeeperMoves() {
    while (state_.player() ==
               state_.description().keeper_player_id() ||
           state_.player() ==
               state_.description().keeper_player_id()) {
      if (state_.player() ==
          state_.description().keeper_player_id()) {
        auto moves = state_.FindMoves(&context_);
        if (moves.empty()) {
          Stop();
          return;
        }
        auto move = moves[rand() % moves.size()];
        state_.MakeMove(move);
        Deliver(Message::ClientMoveMessage(move));
      } else if (state_.player() ==
          state_.description().keeper_player_id()) {
        auto moves = state_.FindFirstMove(&context_);
        if (moves.empty()) {
          Stop();
          return;
        }
        state_.MakeMove(moves[0]);
        Deliver(Message::ClientMoveMessage(moves[0]));
      }
    }
  }
};

class Server {
public:
  explicit Server(const GameDescription &description,
                  boost::asio::io_service &io_service, unsigned short port = 13)
      : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
        game_instance_(description, this), socket_(io_service) {
    DoAccept();
  }

  void DoAccept() {
    acceptor_.async_accept(socket_,
                          [this](boost::system::error_code ec) {
                            if (!ec && !game_instance_.Full()) {
                              auto new_connection = ClientConnection::Create(
                                  std::move(socket_), &game_instance_);
                              new_connection->Start();
                            }
                            if (!game_instance_.Full())
                              DoAccept();
                          });
  }

private:
  tcp::acceptor acceptor_;
  ServerGameInstance game_instance_;

  tcp::socket socket_;
};


#endif //RBGGAMEMANAGER_SERVER_H
