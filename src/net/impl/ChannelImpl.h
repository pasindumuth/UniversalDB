#ifndef UNI_NET_CHANNELIMPL_H
#define UNI_NET_CHANNELIMPL_H

#include <functional>
#include <mutex>
#include <queue>
#include <string>

#include <boost/asio.hpp>

#include <common/common.h>
#include <net/Channel.h>
#include <net/endpoint_id.h>

namespace uni {
namespace net {

class ChannelImpl
    : public uni::net::Channel {
 public:
  ChannelImpl(boost::asio::ip::tcp::socket&& socket);

  uni::net::endpoint_id endpoint_id() override;

  void queue_send(std::string message) override;

  void start_listening() override;

  void set_receive_callback(std::function<bool(std::string)> callback) override;

  void set_close_callback(std::function<void(void)> callback) override;

 private:
  void send(std::string);

  void recv();

  boost::asio::ip::tcp::socket _socket;
  std::queue<std::string> _message_queue;
  std::mutex _queue_lock;
  std::function<bool(std::string)> _receive_callback;
  std::function<void(void)> _close_callback;
};

} // namespace net
} // namespace uni


#endif // UNI_NET_CHANNELIMPL_H
