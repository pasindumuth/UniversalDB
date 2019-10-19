#include "AsyncSchedulerImpl.h"

namespace uni {
namespace async {

using boost::asio::io_context;
using uni::net::IncomingMessage;

AsyncSchedulerImpl::AsyncSchedulerImpl(io_context& io_context)
    : _io_context(io_context),
      _callback([](IncomingMessage){}) {}

void AsyncSchedulerImpl::set_callback(std::function<void(IncomingMessage)> callback) {
  _callback = callback;
}

void AsyncSchedulerImpl::schedule_async(IncomingMessage message) {
  boost::asio::post(_io_context, [this, message](){
    _callback(message);
    std::unique_lock<std::mutex> lock(_queue_lock);
    _message_queue.pop();
    if (_message_queue.size() > 0) {
      schedule_async(_message_queue.front());
    }
  });
}

void AsyncSchedulerImpl::queue_message(IncomingMessage message) {
  std::unique_lock<std::mutex> lock(_queue_lock);
  _message_queue.push(message);
  if (_message_queue.size() == 1) {
    schedule_async(message);
  }
}

} // async
} // uni
