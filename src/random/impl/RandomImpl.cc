#include "RandomImpl.h"

namespace uni {
namespace random {

RandomImpl::RandomImpl()
  : seeder(std::random_device()),
    random_number_engine(std::mt19937(seeder())) {}

std::mt19937& RandomImpl::rng() {
  return random_number_engine;
}

} // namespace random
} // namespace uni
