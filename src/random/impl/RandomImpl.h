#ifndef UNI_RANDOM_RANDOMIMPL_H
#define UNI_RANDOM_RANDOMIMPL_H

#include <random>

#include <random/Random.h>

namespace uni {
namespace random {

class RandomImpl : public uni::random::Random {
 public:
  RandomImpl();

  std::mt19937& rng() override;

 private:
  std::random_device _seeder;
  std::mt19937 _rng;
};

} // namespace random
} // namespace uni

#endif //UNI_RANDOM_RANDOMIMPL_H
