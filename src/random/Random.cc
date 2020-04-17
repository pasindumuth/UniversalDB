#include "Random.h"

namespace uni {
namespace random {

unsigned Random::rand_uniform(unsigned low, unsigned high) {
  std::uniform_int_distribution<std::mt19937::result_type> dist(low, high);
  return dist(rng());
}

} // namespace random
} // namespace uni
