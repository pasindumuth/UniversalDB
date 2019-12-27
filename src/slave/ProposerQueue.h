#ifndef UNI_SLAVE_PROPOSERQUEUE_H
#define UNI_SLAVE_PROPOSERQUEUE_H

#include <functional>
#include <queue>

#include <async/TimerAsyncScheduler.h>
#include <common/common.h>

namespace uni {
namespace slave {

/**
 * @brief A queue of tasks that are run and repeated, based the return value of the task
 * 
 * We can add an arbitrary number of tasks. If the queue is empty, the first task
 * that's added is scheduled to run as soon as possible (but not immediately in the
 * #add_task method call). When a task is run, the return value is used to determine if the task
 * should be run again (when its >= 0), and how far in the future (which is just the return value).
 * If it shouldn't be run again, the task is popped and the next task is scheduled to run as soon
 * as possible (but not immediately in the #add_task method call).
 */
class ProposerQueue {
 public:
  /**
   * @brief Constructs a ProposerQueue, using a TimerAsyncScheduler to schedule the tasks.
   */
  ProposerQueue(
    uni::async::TimerAsyncScheduler& timer_scheduler);

  /**
   * @brief Add a task into the ProposerQueue
   */
  void add_task(std::function<int(void)> callback);

  /**
   * @brief Checks whether the ProposerQueue is empty
   */
  bool empty();

 private:
  void run_task();

  uni::async::TimerAsyncScheduler& _timer_scheduler;

  std::queue<std::function<int(void)>> _callbacks;
};

} // namespace slave
} // namespace uni

#endif // UNI_SLAVE_PROPOSERQUEUE_H
