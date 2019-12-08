#ifndef UNI_ASYNC_TIMERASYNCSCHEDULER_H
#define UNI_ASYNC_TIMERASYNCSCHEDULER_H

#include <functional>

namespace uni {
namespace async {

// Timer that's used to schedule callbacks in the future, either to run once,
// or to run repeatedly. Scheduled callbacks are tasks that run in the foreground
// thread. The wait and period parameters can be zero, but this does not mean that
// the callback is executed while the schedule_once or schedule_repeated method is
// being called. It just means that it will be scheduled in the foreground thread's
// task queue and run as immediately as possible.
class TimerAsyncScheduler {
 public:
  // Schedule the callback to run once in the background after `wait` amount of time.
  virtual void schedule_once(std::function<void(void)> callback, long wait) = 0;

  // Schedule the callback to run repeatedly with period `period`. Note that
  // the first time the callback is run after `wait` amount of time; it doesn't
  // execute immediately.
  virtual void schedule_repeated(std::function<void(void)> callback, long period) = 0;
  
  // Schedule a callback to run several times, each time running after a certain period.
  // This is useful for retrying requests, for instance.
  virtual void schedule_repeated_finite(std::function<void(void)> callback, long period, int tries) = 0;
};

} // namespace async
} // namespace uni

#endif // UNI_ASYNC_TIMERASYNCSCHEDULER_H
