#pragma once

#include <random>

namespace kernel {
namespace random_utils {

std::mt19937& getRng();
int randomInt(int min, int max);

}  // namespace random_utils
}  // namespace kernel