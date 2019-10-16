#include "TimerTesting.h"

namespace uni {
namespace timing {

TimerTesting::TimerTesting() {}

void TimerTesting::schedule_once(std::function<void(void)> callback, unsigned wait) {
  // No-op
}

void TimerTesting::schedule_repeated(std::function<void(void)> callback, unsigned period) {
  // No-op
}

} // namespace timing
} // namespace uni
