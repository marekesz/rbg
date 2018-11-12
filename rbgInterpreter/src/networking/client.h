//
// Created by ??? on 01/02/18.
//

#ifndef RBGGAMEMANAGER_SERVER_CLIENT_H
#define RBGGAMEMANAGER_SERVER_CLIENT_H


#include <boost/asio.hpp>
#include <iostream>
#include "../game_components/search_context.h"
#include "message.h"

using boost::asio::ip::tcp;

class SynchronousClient {

public:
  SynchronousClient(const char *host, const char *port)
      : resolver_(io_service_),
        endpoint_iterator_(resolver_.resolve({host, port})),
        socket_(io_service_) {
    boost::asio::connect(socket_, endpoint_iterator_);
    Initialize();
  }

  void Write(const Move &move) {
    Message message = Message::ClientMoveMessage(move);
    boost::system::error_code ignored_error;
    boost::asio::write(socket_, boost::asio::buffer(message.data_ptr(),
                                                    message.length()),
                       ignored_error);
  }

  const std::string &description() const {
    return game_description_;
  }

  const std::string &player() const {
    return assigned_player_;
  }

  Move Read() {
    Message message = Message::Empty();
    boost::system::error_code ignored_error;
    boost::asio::read(socket_,
                      boost::asio::buffer(message.data_ptr(), kHeaderLength),
                      ignored_error);
    message.DecodeHeader();
    boost::asio::read(socket_, boost::asio::buffer(message.body_ptr(),
                                                   message.body_length()),
                      ignored_error);
    return DecodeMove(message);
  }

private:
  void Initialize() {
    Message message = Message::Empty();
    boost::system::error_code ignored_error;
    boost::asio::read(socket_,
                      boost::asio::buffer(message.data_ptr(), kHeaderLength),
                      ignored_error);
    message.DecodeHeader();
    boost::asio::read(socket_, boost::asio::buffer(message.body_ptr(),
                                                   message.body_length()),
                      ignored_error);
    game_description_ = std::string(message.body_ptr(),
                                    message.body_ptr() + message.body_length());
    boost::asio::read(socket_,
                      boost::asio::buffer(message.data_ptr(), kHeaderLength),
                      ignored_error);
    message.DecodeHeader();
    boost::asio::read(socket_, boost::asio::buffer(message.body_ptr(),
                                                   message.body_length()),
                      ignored_error);
    assigned_player_ = std::string(message.body_ptr(),
                                   message.body_ptr() + message.body_length());
  }

  boost::asio::io_service io_service_;
  tcp::resolver resolver_;
  tcp::resolver::iterator endpoint_iterator_;

  tcp::socket socket_;

  std::string game_description_;
  std::string assigned_player_;
};


#endif //RBGGAMEMANAGER_SERVER_CLIENT_H
