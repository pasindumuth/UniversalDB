#include "ClockTesting.h"

#include <assert/assert.h>
#include <chrono>

namespace uni {
namespace async {
  
ClockTesting::ClockTesting()
    : _time(0) {}

void ClockTesting::increment_time(long increment) {
  _time += increment;
  auto it = _scheduled_callbacks.begin();
  // Iterate through the scheduled callbacks in order (starting from the earliest),
  // and execute the ones which are scheduled to run before the new _time.
  while (it->first <= _time) {
    // Note that _scheduled_callbacks might dynamically change as this iteration
    // is happening. That's why we immediately erase the last etry
    it->second();
    _scheduled_callbacks.erase(it);
    it = _scheduled_callbacks.begin();
  }
}

void ClockTesting::schedule_async(std::function<void(void)> callback, long wait) {
  UNIVERSAL_ASSERT_MESSAGE(wait >= 0, "A clock can only wait on positive amount of time into the future.")
  _scheduled_callbacks.insert({_time + wait, callback});
}

} // namespace async
} // namespace uni
