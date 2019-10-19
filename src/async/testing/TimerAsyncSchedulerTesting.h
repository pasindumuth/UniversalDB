#ifndef UNI_ASYNC_TIMERASYNCSCHEDULERTESTING_H
#define UNI_ASYNC_TIMERASYNCSCHEDULERTESTING_H

#include <functional>

#include <async/TimerAsyncScheduler.h>
#include <async/testing/ClockTesting.h>

namespace uni {
namespace async {

// Testing implementation of a TimerAsyncScheduler.
class TimerAsyncSchedulerTesting : public uni::async::TimerAsyncScheduler {
 public:
  TimerAsyncSchedulerTesting(uni::async::ClockTesting& clock);

  void schedule_once(std::function<void(void)> callback, long wait) override;

  void schedule_repeated(std::function<void(void)> callback, long period) override;

  void schedule_repeated_finite(std::function<void(void)> callback, long period, int tries) override;

 private:
  uni::async::ClockTesting& _clock;
};

} // namespace async
} // namespace uni

#endif // UNI_ASYNC_TIMERASYNCSCHEDULERTESTING_H
