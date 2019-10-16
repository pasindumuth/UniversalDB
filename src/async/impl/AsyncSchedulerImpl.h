#ifndef UNI_ASYNC_ASYNCSCHEDULERIMPL_H
#define UNI_ASYNC_ASYNCSCHEDULERIMPL_H

#include <functional>
#include <string>

#include <boost/asio.hpp>

#include <async/AsyncScheduler.h>
#include <net/IncomingMessage.h>

namespace uni {
namespace async {

// This class wraps an io_context and provides an interface for scheduling it an arbitrary task.
// This task is a function that takes a string, set in this class by calling set_callback. After
// the callback is set, calling schedule_async(message) will schedule a run of that function with the
// argument to the callback being `message`.
class AsyncSchedulerImpl
    : public uni::async::AsyncScheduler {
 public:
  AsyncSchedulerImpl(boost::asio::io_context& io_context);

  void set_callback(std::function<void(uni::net::IncomingMessage)> callback) override;

  void schedule_async(uni::net::IncomingMessage message) override;

 private:
  boost::asio::io_context& _io_context;
  std::function<void(uni::net::IncomingMessage)> _callback;
};

} // async
} // uni

#endif // UNI_ASYNC_ASYNCSCHEDULERIMPL_H
