#include "RandomTesting.h"

namespace uni {
namespace random {

RandomTesting::RandomTesting(unsigned seed)
  : random_number_engine(std::mt19937(seed)) {}

std::mt19937& RandomTesting::rng() {
  return random_number_engine;
}

} // namespace random
} // namespace uni
