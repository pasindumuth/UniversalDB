#ifndef UNI_ASYNC_TIMERASYNCSCHEDULER_H
#define UNI_ASYNC_TIMERASYNCSCHEDULER_H

#include <functional>

#include <common/common.h>

namespace uni {
namespace async {

/**
 * @brief This class can be used to register a callback to run some time in the future.
 * 
 * Timer that's used to schedule callbacks in the future, either to run once,
 * or to run repeatedly. Scheduled callbacks are tasks that run in the foreground
 * thread. They wait and period parameters can be zero, but this does not mean that
 * the callback is executed while the #schedule_once or #schedule_repeated method is
 * being called. It just means that it will be scheduled in the foreground thread's
 * task queue and run as immediately as possible.
 */
class TimerAsyncScheduler {
 public:
  /**
   * @brief Schedule the callback to run once in the background after @p wait amount of time.
   * 
   * If @p wait is 0, then the callback will run as soon as possible, but it won't run during
   * this method call. It will be schedule the callback as a task to be done after the threads
   * current task. #schedule_repeated
   */
  virtual void schedule_once(std::function<void(void)> callback, int64_t wait) = 0;

  /**
   * @brief Schedule the callback to run repeatedly with period @p period .
   * 
   * This method behaves as if #schedule_once was called repeatedly with wait time @p period .
   */
  virtual void schedule_repeated(std::function<void(void)> callback, int64_t period) = 0;
  
  /**
   * @brief Schedule a callback to run several times, each time running after a certain period.
   * 
   * This method behaves as if #schedule_once was called @p tries number of times, each time
   * waiting @p period . This is useful for retrying requests, for instance.
   */
  virtual void schedule_repeated_finite(std::function<void(void)> callback, int64_t period, int32_t tries) = 0;
};

} // namespace async
} // namespace uni

#endif // UNI_ASYNC_TIMERASYNCSCHEDULER_H
