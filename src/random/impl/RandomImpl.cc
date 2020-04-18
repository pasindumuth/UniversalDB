#include "RandomImpl.h"

namespace uni {
namespace random {

RandomImpl::RandomImpl()
  : _seeder(std::random_device()),
    _rng(std::mt19937(_seeder())) {}

std::mt19937& RandomImpl::rng() {
  return _rng;
}

} // namespace random
} // namespace uni
