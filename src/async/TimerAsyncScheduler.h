#ifndef UNI_ASYNC_TIMERASYNCSCHEDULER_H
#define UNI_ASYNC_TIMERASYNCSCHEDULER_H

#include <functional>
#include <memory>

#include <boost/asio.hpp>

namespace uni {
namespace async {

// This class wraps an io_context and provides an interface for scheduling
// arbitrary tasks some time in the future.
class TimerAsyncScheduler {
 public:
  TimerAsyncScheduler(std::shared_ptr<boost::asio::io_context> io_context);

  void schedule_deferred(long duration_ms, std::function<void(void)> callback);

 private:
  std::shared_ptr<boost::asio::io_context> _io_context;
};

} // async
} // uni


#endif // UNI_ASYNC_TIMERASYNCSCHEDULER_H
