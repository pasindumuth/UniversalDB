#ifndef UNI_ASYNC_ASYNCSCHEDULER_H
#define UNI_ASYNC_ASYNCSCHEDULER_H

#include <functional>

#include <net/IncomingMessage.h>

namespace uni {
namespace async {

class AsyncScheduler {
 public:
  virtual void set_callback(std::function<void(uni::net::IncomingMessage)> callback) = 0;

  virtual void queue_message(uni::net::IncomingMessage message) = 0;
};

} // async
} // uni


#endif // UNI_ASYNC_ASYNCSCHEDULER_H
