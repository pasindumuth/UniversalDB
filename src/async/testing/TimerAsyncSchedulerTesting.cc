#include "TimerAsyncSchedulerTesting.h"

namespace uni {
namespace async {

using uni::async::ClockTesting;

TimerAsyncSchedulerTesting::TimerAsyncSchedulerTesting(ClockTesting& clock)
    : _clock(clock) {}

void TimerAsyncSchedulerTesting::schedule_once(std::function<void(void)> callback, long wait) {
  _clock.schedule_async(callback, wait);
}

void TimerAsyncSchedulerTesting::schedule_repeated(std::function<void(void)> callback, long period) {
  _clock.schedule_async([this, callback, period]() {
    callback();
    schedule_repeated(callback, period);
  }, period);
}

void TimerAsyncSchedulerTesting::schedule_repeated_finite(std::function<void(void)> callback, long period, int tries) {
  _clock.schedule_async([this, callback, period, tries]() {
    callback();
    if (tries > 1) {
      schedule_repeated_finite(callback, period, tries - 1);
    }
  }, period);
}

} // namespace async
} // namespace uni
