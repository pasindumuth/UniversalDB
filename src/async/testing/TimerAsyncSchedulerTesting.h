#ifndef UNI_ASYNC_TIMERASYNCSCHEDULERTESTING_H
#define UNI_ASYNC_TIMERASYNCSCHEDULERTESTING_H

#include <functional>

#include <async/TimerAsyncScheduler.h>

namespace uni {
namespace async {

// Testing implementation of a TimerAsyncScheduler.
class TimerAsyncSchedulerTesting : public uni::async::TimerAsyncScheduler {
 public:
  TimerAsyncSchedulerTesting();

  void schedule_once(std::function<void(void)> callback, unsigned wait) override;

  void schedule_repeated(std::function<void(void)> callback, unsigned period) override;
};

} // namespace async
} // namespace uni

#endif // UNI_ASYNC_TIMERASYNCSCHEDULERTESTING_H
