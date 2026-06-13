#include "random_utils.h"

#include <random>

namespace kernel {
namespace random_utils {

std::mt19937& getRng() {
  static std::mt19937 rng(std::random_device{}());
  return rng;
}

int randomInt(int min, int max) {
  std::uniform_int_distribution<int> dist(min, max);
  return dist(getRng());
}

}  // namespace random_utils
}  // namespace kernel