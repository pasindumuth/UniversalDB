#include "TimerAsyncSchedulerTesting.h"

namespace uni {
namespace async {

TimerAsyncSchedulerTesting::TimerAsyncSchedulerTesting() {}

void TimerAsyncSchedulerTesting::schedule_once(std::function<void(void)> callback, unsigned wait) {
  // No-op
}

void TimerAsyncSchedulerTesting::schedule_repeated(std::function<void(void)> callback, unsigned period) {
  // No-op
}

} // namespace async
} // namespace uni
