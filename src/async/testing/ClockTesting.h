#ifndef UNI_ASYNC_CLOCKTESTING_H
#define UNI_ASYNC_CLOCKTESTING_H

#include <functional>
#include <map>

namespace uni {
namespace async {

/**
 * @brief This class is a clock used for testing. The units of time are milliseconds.
 */
class ClockTesting {
 public:
  ClockTesting();

  /**
   * @brief Increments the time and executes callbacks upto and including the new time.
   * 
   * All callbacks whose deadlines which are less than or equal to the new time are
   * executed. Note that as a callback executes, it may schedule a new task that
   * is before the new clock's time as well. This too will be executed.
   */
  void increment_time(long increment);

  /**
   * @brief Schedules a callback to run @p wait milliseconds into the future.
   * 
   * If there are already callbacks that need to be run at that time in the future,
   * this new callback will run after all those finish running.
   */
  void schedule_async(std::function<void(void)> callback, long wait);

 private:
  long _time; // time in milliseconds
  std::multimap<long, std::function<void(void)>> _scheduled_callbacks;
};

} // namespace async
} // namespace uni

#endif // UNI_ASYNC_CLOCKTESTING_H
