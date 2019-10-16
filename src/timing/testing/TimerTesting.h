#ifndef UNI_TIMING_TIMERTESTING_H
#define UNI_TIMING_TIMERTESTING_H

#include <functional>

#include <timing/Timer.h>

namespace uni {
namespace timing {

// Testing implementation of a timer.
class TimerTesting : public uni::timing::Timer {
 public:
  TimerTesting();

  void schedule_once(std::function<void(void)> callback, unsigned wait) override;

  void schedule_repeated(std::function<void(void)> callback, unsigned period) override;
};

} // namespace timing
} // namespace uni

#endif // UNI_TIMING_TIMERTESTING_H
