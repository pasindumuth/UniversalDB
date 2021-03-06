#include "TimerAsyncSchedulerImpl.h"

#include <memory>
#include <time.h>

#include <assert/assert.h>

namespace uni {
namespace async {

TimerAsyncSchedulerImpl::TimerAsyncSchedulerImpl(boost::asio::io_context& io_context)
  : _io_context(io_context) {}

void TimerAsyncSchedulerImpl::schedule_once(std::function<void(void)> callback, int64_t wait) {
  auto t = std::make_shared<boost::asio::steady_timer>(_io_context, boost::asio::chrono::milliseconds(wait));
  t->async_wait([callback, t](const boost::system::error_code& ec){
    UNIVERSAL_ASSERT_MESSAGE(!ec, ec.message())
    callback();
  });
}

void TimerAsyncSchedulerImpl::schedule_repeated(std::function<void(void)> callback, int64_t period) {
  auto t = std::make_shared<boost::asio::steady_timer>(_io_context, boost::asio::chrono::milliseconds(period));
  t->async_wait([this, t, callback, period](const boost::system::error_code& ec){
    UNIVERSAL_ASSERT_MESSAGE(!ec, ec.message())
    callback();
    // Schedule this callback to be executed again after a `period`.
    schedule_repeated(callback, period);
  });
}

void TimerAsyncSchedulerImpl::schedule_repeated_finite(std::function<void(void)> callback, int64_t period, int32_t tries) {
  auto t = std::make_shared<boost::asio::steady_timer>(_io_context, boost::asio::chrono::milliseconds(period));
  t->async_wait([this, t, callback, period, tries](const boost::system::error_code& ec){
    UNIVERSAL_ASSERT_MESSAGE(!ec, ec.message())
    callback();
    if (tries > 1) {
      // Schedule this callback to be executed again after a `period`.
      schedule_repeated_finite(callback, period, tries - 1);
    }
  });
}

} // namespace async
} // namespace uni
