#include "AsyncSchedulerImpl.h"

namespace uni {
namespace async {

using boost::asio::io_context;
using uni::net::IncomingMessage;

AsyncSchedulerImpl::AsyncSchedulerImpl(std::shared_ptr<io_context> io_context)
    : _io_context(io_context),
      _callback([](IncomingMessage){}) {}

void AsyncSchedulerImpl::set_callback(std::function<void(IncomingMessage)> callback) {
  _callback = callback;
}

void AsyncSchedulerImpl::schedule_async(IncomingMessage message) {
  boost::asio::post(*_io_context, [this, message](){
    _callback(message);
  });
}

} // async
} // uni
