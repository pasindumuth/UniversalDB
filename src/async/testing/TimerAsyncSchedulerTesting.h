#ifndef UNI_ASYNC_TIMERASYNCSCHEDULERTESTING_H
#define UNI_ASYNC_TIMERASYNCSCHEDULERTESTING_H

#include <functional>

#include <async/TimerAsyncScheduler.h>
#include <async/testing/ClockTesting.h>

namespace uni {
namespace async {

/**
 * @brief An implementation of TimerAsyncScheduler that is used for testing.
 * 
 * It uses ClockTesting to simulate the clock. When callbacks are scheduled,
 * they are scheduled according to the current time of the clock, and when
 * the clock is incremented, any callbacks whose scheduled time lies after
 * the previous time and before the next time will be executed.
 */
class TimerAsyncSchedulerTesting : public uni::async::TimerAsyncScheduler {
 public:
  /**
   * @brief Constructs a new async scheduler that uses the provided clock for scheduling.
   */
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
