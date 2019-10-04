#include "TimerAsyncScheduler.h"

#include <assert/assert.h>

namespace uni {
namespace async {

TimerAsyncScheduler::TimerAsyncScheduler(
    std::shared_ptr<boost::asio::io_context> io_context)
      : _io_context(io_context) {}

void TimerAsyncScheduler::schedule_deferred(long duration_ms, std::function<void(void)> callback) {
  auto t = std::make_shared<boost::asio::steady_timer>(*_io_context, boost::asio::chrono::milliseconds(duration_ms));
  t->async_wait([callback, t](const boost::system::error_code& ec){
    UNIVERSAL_ASSERT_MESSAGE(!ec, ec.message())
    callback();
  });
}

} // async
} // uni
