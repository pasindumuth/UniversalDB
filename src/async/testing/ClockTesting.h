#ifndef UNI_ASYNC_CLOCKTESTING_H
#define UNI_ASYNC_CLOCKTESTING_H

#include <functional>
#include <map>

namespace uni {
namespace async {

// Clock used for testing.
class ClockTesting {
 public:
  ClockTesting();

  // Increments the clock's time by an amount `increment`. All callbacks 
  // whose deadline lies within the window from where the old time was to
  // where the new time is now is executed.
  void increment_time(long increment);

  // Schedules the callback to execute after some time `wait`
  void schedule_async(std::function<void(void)> callback, long wait);

 private:
  long _time; // time in milliseconds
  std::map<long, std::function<void(void)>> _scheduled_callbacks;
};

} // namespace async
} // namespace uni

#endif // UNI_ASYNC_CLOCKTESTING_H
