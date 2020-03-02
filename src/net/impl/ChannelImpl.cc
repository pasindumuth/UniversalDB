#include "ChannelImpl.h"

#include <iostream>
#include <stdlib.h>
#include <vector>

#include <net/constants.h>

namespace uni {
namespace net {

using boost::asio::ip::tcp;

ChannelImpl::ChannelImpl(tcp::socket&& socket)
    : _socket(std::move(socket)) {}

void ChannelImpl::queue_send(std::string message) {
  std::unique_lock<std::mutex> lock(_queue_lock);
  if (_message_queue.size() < MAX_MESSAGES_QUEUE_SIZE) {
    _message_queue.push(message);
    if (_message_queue.size() == 1) {
      send(message);
    }
  }
}

endpoint_id ChannelImpl::endpoint_id() {
  auto const& ip_string = _socket.remote_endpoint().address().to_string();
  auto const& port = _socket.remote_endpoint().port();
  return uni::net::endpoint_id(ip_string, port);
}

void ChannelImpl::start_listening() {
  recv();
}

// Implement timeouts, retries, and safety on remote socket closure.
void ChannelImpl::send(std::string message) {
  auto length = message.size();
  boost::asio::async_write(_socket, boost::asio::buffer(INT_TO_STR(length)),
      [this, message](const boost::system::error_code&, size_t bytes_transferred) {
   LOG(uni::logging::Level::TRACE2, "Header bytes sent: " + std::to_string(bytes_transferred))
    boost::asio::async_write(_socket, boost::asio::buffer(message),
        [this](const boost::system::error_code&, size_t bytes_transferred) {
     LOG(uni::logging::Level::TRACE2, "Body bytes sent: " + std::to_string(bytes_transferred))
      std::unique_lock<std::mutex> lock(_queue_lock);
      _message_queue.pop();
      if (_message_queue.size() > 0) {
        send(_message_queue.front());
      }
    });
  });
}

void ChannelImpl::recv() {
  void* header_buf = malloc(4);
  boost::asio::async_read(_socket, boost::asio::buffer(header_buf, 4),
      [this, header_buf](const boost::system::error_code&, size_t bytes_transferred) {
   LOG(uni::logging::TRACE2, "Header bytes received: " + std::to_string(bytes_transferred))
    if (bytes_transferred == 0) {
     LOG(uni::logging::TRACE2, "Remove socket closed")
      free(header_buf);
      for (auto callback: _close_callbacks) {
        callback();
      }
    } else {
      auto length = *((int*) header_buf);
      free(header_buf);
      void* buf = malloc(length);
      boost::asio::async_read(_socket, boost::asio::buffer(buf, length),
          [this, buf, length](const boost::system::error_code&, size_t bytes_transferred) {
       LOG(uni::logging::TRACE2, "Body bytes received: " + std::to_string(bytes_transferred))
        std::string serialized((char*) buf, length);
        free(buf);
        for (auto callback: _receive_callbacks) {
          if (!callback(serialized)) {
            // If even one callback returns false, we use that as an indication to
            // stop running the rest of the callbacks and to stop listening for
            // more data in the socket.
            return;
          }
        }
        recv();
      });
    }
  });
}

} // namespace net
} // namespace uni
