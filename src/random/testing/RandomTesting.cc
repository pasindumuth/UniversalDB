#include "RandomTesting.h"

namespace uni {
namespace random {

RandomTesting::RandomTesting(unsigned seed)
  : _rng(std::mt19937(seed)) {}

std::mt19937& RandomTesting::rng() {
  return _rng;
}

} // namespace random
} // namespace uni
