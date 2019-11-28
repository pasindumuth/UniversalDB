#ifndef UNI_SLAVE_PROPOSERQUEUE_H
#define UNI_SLAVE_PROPOSERQUEUE_H

#include <functional>
#include <queue>

#include <async/TimerAsyncScheduler.h>

namespace uni {
namespace slave {

class ProposerQueue {
 public:
  ProposerQueue(
    uni::async::TimerAsyncScheduler& timer_scheduler);

  void add_task(std::function<int(void)> callback);

  void run_task();

  bool empty();

 private:
  uni::async::TimerAsyncScheduler& _timer_scheduler;

  std::queue<std::function<int(void)>> _callbacks;
};

} // namespace slave
} // namespace uni

#endif // UNI_SLAVE_PROPOSERQUEUE_H
