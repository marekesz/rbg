//
// Created by ??? on 31/01/18.
//

#include "server.h"

void ClientConnection::Write() {
  auto self(shared_from_this());
  boost::asio::async_write(socket_,
                           boost::asio::buffer(write_moves_.front().data_ptr(),
                                               write_moves_.front().length()),
                           [this, self](boost::system::error_code ec,
                                        std::size_t /*length*/) {
                             if (!ec) {
                               write_moves_.pop_front();
                               if (!write_moves_.empty()) {
                                 Write();
                               }
                             } else {
                               instance_->Leave(shared_from_this());
                             }
                           });
}

void ClientConnection::ReadBody() {
  auto self(shared_from_this());
  boost::asio::async_read(socket_,
                          boost::asio::buffer(read_move_.body_ptr(),
                                              read_move_.body_length()),
                          [this, self](boost::system::error_code ec,
                                       std::size_t /*length*/) {
                            bool valid = instance_->HandleMove(
                                shared_from_this(), read_move_);
                            if (valid && !ec) {
                              ReadHeader();
                            } else {
                              instance_->Leave(shared_from_this());
                            }
                          });
}

void ClientConnection::ReadHeader() {
  auto self(shared_from_this());
  boost::asio::async_read(socket_, boost::asio::buffer(read_move_.data_ptr(),
                                                       kHeaderLength),
                          [this, self](boost::system::error_code ec,
                                       std::size_t /*length*/) {
                            if (!ec && read_move_.DecodeHeader())
                              ReadBody();
                            else
                              instance_->Leave(shared_from_this());
                          });
}

void ClientConnection::Start() {
  instance_->Join(shared_from_this());
  ReadHeader();
}

void ServerGameInstance::Leave(const ClientConnection::pointer &client) {
  /* TODO(???): Figure out what to do with other clients when someone leaves.
   * */
  std::cout << client << " left_ the game." << std::endl;
  token_id_t player_token = clients_[client];
  clients_.erase(client);
  players_[player_token] = nullptr;
  state_.Reset();
  if (controlling_server_)
    controlling_server_->DoAccept();
}
