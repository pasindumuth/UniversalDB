#ifndef UNI_TIMING_TIMERIMPL_H
#define UNI_TIMING_TIMERIMPL_H

#include <functional>

#include <boost/asio.hpp>

#include <timing/Timer.h>

namespace uni {
namespace timing {

// Implementation of a timer that uses boost::asio to schedule
// the deferred callbacks. This is used for production.
class TimerImpl : public uni::timing::Timer {
 public:
  TimerImpl(boost::asio::io_context& io_context);
  
  void schedule_once(std::function<void(void)> callback, unsigned wait) override;

  void schedule_repeated(std::function<void(void)> callback, unsigned period) override;

 private:
  boost::asio::io_context& _io_context;
};

} // namespace timing
} // namespace uni

#endif // UNI_TIMING_TIMERIMPL_H
