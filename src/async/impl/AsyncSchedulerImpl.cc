#include "AsyncSchedulerImpl.h"

namespace uni {
namespace async {

AsyncSchedulerImpl::AsyncSchedulerImpl(boost::asio::io_context& io_context)
    : _io_context(io_context),
      _callback([](uni::net::IncomingMessage){}) {}

void AsyncSchedulerImpl::set_callback(std::function<void(uni::net::IncomingMessage)> callback) {
  _callback = callback;
}

void AsyncSchedulerImpl::schedule_async(uni::net::IncomingMessage message) {
  boost::asio::post(_io_context, [this, message](){
    _callback(message);
    std::unique_lock<std::mutex> lock(_queue_lock);
    _message_queue.pop();
    if (_message_queue.size() > 0) {
      schedule_async(_message_queue.front());
    }
  });
}

void AsyncSchedulerImpl::queue_message(uni::net::IncomingMessage message) {
  std::unique_lock<std::mutex> lock(_queue_lock);
  _message_queue.push(message);
  if (_message_queue.size() == 1) {
    schedule_async(message);
  }
}

} // async
} // uni
