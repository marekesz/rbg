//
// Created by ??? on 15.01.18.
//

#ifndef RBGGAMEMANAGER_FAST_RESET_VECTOR_H
#define RBGGAMEMANAGER_FAST_RESET_VECTOR_H

#include <vector>
#include <limits>

typedef unsigned int uint;

// This is vector of booleans supporting fast reset function. Reset
// is done in constant time, independent from the size of the vector.
// Example:
//   FastBitVector vec(10, false);
//   vec[5] = true;
//   vec.reset();
class FastBitVector {
public:
  FastBitVector(std::size_t size, bool value)
      : data_(size, (unsigned int) value) {
    threshold_ = 1;
  }

  bool operator[](const uint index) const {
    return data_[index] >= threshold_;
  }

  void set(const uint index) {
    data_[index] = threshold_;
  }

  void reset() {
    if (threshold_ == std::numeric_limits<unsigned int>::max()) {
      std::fill(data_.begin(), data_.end(), 0);
      threshold_ = 1;
    }
    threshold_++;
  }

  void resize(std::size_t new_size) {
    data_.resize(new_size);
  }

  std::size_t size() const {
    return data_.size();
  }

private:
  std::vector<unsigned int> data_;
  unsigned int threshold_;
};


#endif //RBGGAMEMANAGER_FAST_RESET_VECTOR_H
