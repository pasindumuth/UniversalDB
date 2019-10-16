#ifndef UNI_ASYNC_TIMERASYNCSCHEDULERIMPL_H
#define UNI_ASYNC_TIMERASYNCSCHEDULERIMPL_H

#include <functional>

#include <boost/asio.hpp>

#include <async/TimerAsyncScheduler.h>

namespace uni {
namespace async {

// Implementation of a TimerAsyncScheduler that uses boost::asio to schedule
// the deferred callbacks. This is used for production.
class TimerAsyncSchedulerImpl : public uni::async::TimerAsyncScheduler {
 public:
  TimerAsyncSchedulerImpl(boost::asio::io_context& io_context);
  
  void schedule_once(std::function<void(void)> callback, long wait) override;

  void schedule_repeated(std::function<void(void)> callback, long period) override;

 private:
  boost::asio::io_context& _io_context;
};

} // namespace async
} // namespace uni

#endif // UNI_ASYNC_TIMERASYNCSCHEDULERIMPL_H
