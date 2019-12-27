#include "ClockTesting.h"

#include <assert/assert.h>
#include <chrono>
#include <logging/log.h>

namespace uni {
namespace async {
  
ClockTesting::ClockTesting()
    : _time(0) {}

void ClockTesting::increment_time(long increment) {
  for (auto i = 0; i < increment; i++) {
    // We increment _time progressively (rather than all at once) so that if
    // schedule_async is called during one of the callbacks in _scheduled_callbacks,
    // then it's scheduled into the future properly.
    _time++;
    auto it = _scheduled_callbacks.begin();
    // Iterate through the scheduled callbacks in order (starting from the earliest),
    // and execute the ones which are scheduled to run before the new _time.
    while (it != _scheduled_callbacks.end() && it->first <= _time) {
      // Note that _scheduled_callbacks might dynamically change as this iteration
      // is happening. That's why we immediately erase the last entry
      it->second();
      // We have to call this again to get back the first element _scheduled_callbacks,
      // since _scheduled_callbacks might have changed while the first callback was running.
      it = _scheduled_callbacks.begin();
      _scheduled_callbacks.erase(it);
      it = _scheduled_callbacks.begin();
    }
  }
}

void ClockTesting::schedule_async(std::function<void(void)> callback, long wait) {
  UNIVERSAL_ASSERT_MESSAGE(wait >= 0, "A clock can only wait on positive amount of time into the future.")
  _scheduled_callbacks.insert({_time + wait, callback});
}

} // namespace async
} // namespace uni
