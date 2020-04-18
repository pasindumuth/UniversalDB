#ifndef UNI_RANDOM_RANDOMIMPL_H
#define UNI_RANDOM_RANDOMIMPL_H

#include <random>

#include <random/Random.h>

namespace uni {
namespace random {

class RandomTesting : public uni::random::Random {
 public:
  RandomTesting(unsigned seed);

  std::mt19937& rng() override;

 private:
  std::mt19937 _rng;
};

} // namespace random
} // namespace uni

#endif //UNI_RANDOM_RANDOMIMPL_H
