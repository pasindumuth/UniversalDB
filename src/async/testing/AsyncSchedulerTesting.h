#ifndef UNI_ASYNC_ASYNCSCHEDULERTESTING_H
#define UNI_ASYNC_ASYNCSCHEDULERTESTING_H

#include <functional>

#include <async/AsyncScheduler.h>
#include <net/IncomingMessage.h>

namespace uni {
namespace async {

// This test implementation works where schedule_async runs the callback with the
// incoming message immediately.
class AsyncSchedulerTesting
    : public uni::async::AsyncScheduler {
 public:
  AsyncSchedulerTesting();

  void set_callback(std::function<void(uni::net::IncomingMessage)> callback) override;

  void queue_message(uni::net::IncomingMessage message) override;

 private:
  std::function<void(uni::net::IncomingMessage)> _callback;
};

} // async
} // uni

#endif // UNI_ASYNC_ASYNCSCHEDULERTESTING_H
