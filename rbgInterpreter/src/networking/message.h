//
// Created by ??? on 01/02/18.
//

#ifndef RBGGAMEMANAGER_MESSAGE_H
#define RBGGAMEMANAGER_MESSAGE_H


#include <string>
#include <cstring>
#include "networking_utils.h"
#include "../game_components/search_context.h"

constexpr std::size_t kHeaderLength = 16;
constexpr std::size_t kMaxBodyLength = 65536;

class Message {
public:
  Message(const std::string &str) {
    body_length_ = std::min(str.length(), kMaxBodyLength);
    std::memcpy(body_ptr(), str.data(), body_length());
    EncodeHeader();
  }

  static Message Empty() {
    return {};
  }

  static Message ClientMoveMessage(const Move &move) {
    std::string move_string = MoveDataString(move);
    return {move_string};
  }

  bool DecodeHeader() {
    char header[kHeaderLength + 1] = "";
    std::strncat(header, data_, kHeaderLength);
    body_length_ = static_cast<size_t>(std::atoi(header));
    if (body_length_ > kMaxBodyLength) {
      body_length_ = 0;
      return false;
    }
    return true;
  }

  char *data_ptr() {
    return data_;
  }

  char *body_ptr() {
    return data_ + kHeaderLength;
  }

  const char *body_ptr() const {
    return data_ + kHeaderLength;
  }

  std::size_t body_length() const {
    return body_length_;
  }

  std::size_t length() const {
    return kHeaderLength + body_length_;
  }

private:
  void EncodeHeader() {
    char header[kHeaderLength + 1] = "";
    std::sprintf(header, "%8lu", body_length_);
    std::memcpy(data_, header, kHeaderLength);
  }

  Message() = default;

  char data_[kHeaderLength + kMaxBodyLength];
  std::size_t body_length_;
};


#endif //RBGGAMEMANAGER_MESSAGE_H
