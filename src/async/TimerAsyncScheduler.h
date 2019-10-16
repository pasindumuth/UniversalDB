#ifndef UNI_ASYNC_TIMERASYNCSCHEDULER_H
#define UNI_ASYNC_TIMERASYNCSCHEDULER_H

#include <functional>

namespace uni {
namespace async {

// Timer that's used to schedule callbacks in the future, either to run once,
// or to run repeatedly.
class TimerAsyncScheduler {
 public:
  // Schedule the callback to run once in the background after `wait` amount of time.
  virtual void schedule_once(std::function<void(void)> callback, unsigned wait) = 0;

  // Schedul the callback to run repeatedly with period `period`. Note that
  // the first time the callback is run after `wait` amount of time; it doesn't
  // execute immediately.
  virtual void schedule_repeated(std::function<void(void)> callback, unsigned period) = 0;
};

} // namespace async
} // namespace uni

#endif // UNI_ASYNC_TIMERASYNCSCHEDULER_H
