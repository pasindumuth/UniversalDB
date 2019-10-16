#include "TimerImpl.h"

#include <memory>
#include <time.h>

#include <assert/assert.h>

namespace uni {
namespace timing {

TimerImpl::TimerImpl(boost::asio::io_context& io_context)
    : _io_context(io_context) {}

void TimerImpl::schedule_once(std::function<void(void)> callback, unsigned wait) {
  auto t = std::make_shared<boost::asio::steady_timer>(_io_context, boost::asio::chrono::milliseconds(wait));
  t->async_wait([callback, t](const boost::system::error_code& ec){
    UNIVERSAL_ASSERT_MESSAGE(!ec, ec.message())
    callback();
  });
}

void TimerImpl::schedule_repeated(std::function<void(void)> callback, unsigned period) {
  auto t = std::make_shared<boost::asio::steady_timer>(_io_context, boost::asio::chrono::milliseconds(period));
  t->async_wait([this, t, callback, period](const boost::system::error_code& ec){
    UNIVERSAL_ASSERT_MESSAGE(!ec, ec.message())
    callback();
    // Schedule this callback to be executed again after a `period`.
    schedule_repeated(callback, period);
  });
}

} // namespace timing
} // namespace uni
