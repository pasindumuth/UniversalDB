#ifndef UNI_NET_CHANNEL_H
#define UNI_NET_CHANNEL_H

#include <functional>
#include <string>
#include <vector>

#include <common/common.h>
#include <net/EndpointId.h>

namespace uni {
namespace net {

// This class provides a simple interface to the network. We use the
// Semi-FIFO Network model. This class represents an interface for both the
// In-Semi-FIFO Channel as well as the Out-Semi-FIFO Channel.
class Channel {
 public:
  virtual ~Channel() {};

  // The endpoint of the other end of the channel.
  // The port is just set to 0; only the ip address matters.
  virtual uni::net::EndpointId endpoint_id() = 0;

  // Queues a message to be send to the other end of the channel.
  virtual void queue_send(std::string message) = 0;

  // No recieve callbacks run until this method is called. Calling this method
  // starts the process of listening to the network for messages.
  virtual void start_listening() = 0;

  // Registers a callback to run when data is received from the other of the channel.
  // The callback returns a boolean indicating whether to continue running that callback
  // when data is received; i.e. if false is returned, then the callback stops being run.
  void add_receive_callback(std::function<bool(std::string)> callback);

  // Registers a callback when a closing message is sent from the other side.
  void add_close_callback(std::function<void(void)> callback);

 protected:
  std::vector<std::function<bool(std::string)>> _receive_callbacks;
  std::vector<std::function<void(void)>> _close_callbacks;
};

} // namespace net
} // namespace uni


#endif // UNI_NET_CHANNEL_H
