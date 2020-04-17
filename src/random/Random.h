#ifndef UNI_RANDOM_RANDOM_H
#define UNI_RANDOM_RANDOM_H

#include <random>

namespace uni {
namespace random {

class Random {
 public:
  virtual ~Random() {};

  virtual std::mt19937& rng() = 0;

  // Generates a random number in the range [low, high] using the rng
  unsigned rand_uniform(unsigned low, unsigned high);
};

} // namespace random
} // namespace uni

#endif //UNI_RANDOM_RANDOM_H
