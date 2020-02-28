#include "AsyncSchedulerTesting.h"

namespace uni {
namespace async {

using uni::net::IncomingMessage;

AsyncSchedulerTesting::AsyncSchedulerTesting()
    : _callback([](uni::net::IncomingMessage){}) {}

void AsyncSchedulerTesting::set_callback(std::function<void(uni::net::IncomingMessage)> callback) {
  _callback = callback;
}

void AsyncSchedulerTesting::queue_message(uni::net::IncomingMessage message) {
  _callback(message);
}

} // async
} // uni
