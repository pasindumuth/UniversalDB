#include "ProposerQueue.h"

namespace uni {
namespace slave {

ProposerQueue::ProposerQueue(
    uni::async::TimerAsyncScheduler& timer_scheduler)
      : _timer_scheduler(timer_scheduler) {}

void ProposerQueue::add_task(std::function<int(void)> callback) {
  _callbacks.push(callback);
  if (_callbacks.size() == 1) {
    _timer_scheduler.schedule_once([this](){ run_task(); }, 0);
  }
}

void ProposerQueue::run_task() {
  auto const& callback = _callbacks.front();
  auto wait = callback();
  if (wait < 0) {
    _callbacks.pop();
    if (_callbacks.size() > 0) {
      _timer_scheduler.schedule_once([this](){ run_task(); }, 0);
    }
  } else {
    _timer_scheduler.schedule_once([this](){ run_task(); }, wait);
  }
}

bool ProposerQueue::empty() {
  return _callbacks.empty();
}

} // namespace slave
} // namespace uni
