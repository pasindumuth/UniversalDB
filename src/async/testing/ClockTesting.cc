#include "ClockTesting.h"

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
  while (it->first < _time) {
    it->second();
    it = std::next(it);
  }
  // Remove all callbacks that were just executed
  _scheduled_callbacks.erase(_scheduled_callbacks.begin(), it);
}

void ClockTesting::schedule_async(std::function<void(void)> callback, long wait) {
  _scheduled_callbacks.insert({_time + wait, callback});
}

} // namespace async
} // namespace uni
