#include "ChannelImpl.h"

#include <iostream>
#include <stdlib.h>
#include <vector>

#include <logging/log.h>
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

// Only meant to be called once before start_listening()
void ChannelImpl::set_recieve_callback(std::function<bool(std::string)> callback) {
  _recieve_callback = callback;
}

// Only meant to be called once before start_listening()
void ChannelImpl::set_close_callback(std::function<void(void)> callback) {
  _close_callback = callback;
}

// Implement timeouts, retries, and safety on remote socket closure.
void ChannelImpl::send(std::string message) {
  int length = message.size();
  boost::asio::async_write(_socket, boost::asio::buffer(INT_TO_STR(length)),
      [this, message](const boost::system::error_code&, size_t bytes_transferred) {
   LOG(uni::logging::Level::TRACE, "Header bytes sent: " + std::to_string(bytes_transferred))
    boost::asio::async_write(_socket, boost::asio::buffer(message),
        [this](const boost::system::error_code&, size_t bytes_transferred) {
     LOG(uni::logging::Level::TRACE, "Body bytes sent: " + std::to_string(bytes_transferred))
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
   LOG(uni::logging::TRACE, "Header bytes received: " + std::to_string(bytes_transferred))
    if (bytes_transferred == 0) {
     LOG(uni::logging::TRACE, "Remove socket closed")
      free(header_buf);
      _close_callback();
    } else {
      int length = *((int*) header_buf);
      free(header_buf);
      void* buf = malloc(length);
      boost::asio::async_read(_socket, boost::asio::buffer(buf, length),
          [this, buf, length](const boost::system::error_code&, size_t bytes_transferred) {
       LOG(uni::logging::TRACE, "Body bytes received: " + std::to_string(bytes_transferred))
        std::string serialized((char*) buf, length);
        free(buf);
        if (_recieve_callback(serialized)) {
          recv();
        }
      });
    }
  });
}

} // namespace net
} // namespace uni
