#include "AsyncSchedulerTesting.h"

namespace uni {
namespace async {

using uni::net::IncomingMessage;

AsyncSchedulerTesting::AsyncSchedulerTesting()
    : _callback([](IncomingMessage){}) {}

void AsyncSchedulerTesting::set_callback(std::function<void(IncomingMessage)> callback) {
  _callback = callback;
}

void AsyncSchedulerTesting::queue_message(IncomingMessage message) {
  _callback(message);
}

} // async
} // uni
