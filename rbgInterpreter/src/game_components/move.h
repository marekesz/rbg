//
// Created by ??? on 21/02/18.
//

#ifndef RBGGAMEMANAGER_MOVE_H
#define RBGGAMEMANAGER_MOVE_H

#include <vector>
#include <boost/container/small_vector.hpp>

using vertex_t = ssize_t;

class BlockApplication {
public:
  BlockApplication(std::size_t applied_vertex,
                   std::size_t block_id)
      : applied_vertex_(applied_vertex),
        block_id_(block_id) {}


  vertex_t pos() const
  {
    return applied_vertex_;
  }

  std::size_t id() const {
    return block_id_;
  }

  bool operator==(const BlockApplication &b) const {
    return pos() == b.pos() && id() == b.id();
  }


  bool operator!=(const BlockApplication &b) const {
    return pos() != b.pos() || id() != b.id();
  }

private:
  vertex_t applied_vertex_;
  std::size_t block_id_;
};

class Move {
public:
  Move() = default;

  template<typename ...Args>
  void AddBlock(Args &&... args) {
    blocks_.emplace_back(std::forward<Args>(args)...);
  }

  void PopBlock() {
    blocks_.pop_back();
  }

  //const std::vector<BlockApplication> &blocks() const {
  const boost::container::small_vector<BlockApplication,8> &blocks() const {
    return blocks_;
  }

  bool operator==(const Move &b) const {
    const auto &a_blocks = blocks();
    const auto &b_blocks = b.blocks();
    if (a_blocks.size() != b_blocks.size())
      return false;
    for (std::size_t i = 0; i < a_blocks.size(); i++) {
      if (a_blocks[i] != b_blocks[i]) {
        return false;
      }
    }
    return true;
  }

  bool operator!=(const Move &b) const {
    return !(*this == b);
  }

private:
  //std::vector<BlockApplication> blocks_;
  boost::container::small_vector<BlockApplication,8> blocks_;
};

namespace std {
  template<>
  struct hash<Move> {
    std::size_t operator()(const Move &m) const {
      std::size_t result = 0;
      for (const auto &block : m.blocks()) {
        result *= 2579;
        result += (block.pos()) * 83 + block.id();
      }
      return result;
    }
  };
}

#endif //RBGGAMEMANAGER_MOVE_H
