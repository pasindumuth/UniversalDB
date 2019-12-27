#ifndef UNI_NET_CHANNEL_H
#define UNI_NET_CHANNEL_H

#include <functional>
#include <string>

#include <net/endpoint_id.h>

namespace uni {
namespace net {

/**
 * This class provides a simple interface to the network. Recall from
 * http://localhost:3000/projects/universaldb/networkmodel that we use the
 * Semi-FIFO Network model. This class represents an interface for both the
 * In-Semi-FIFO Channel as well as the Out-Semi-FIFO Channel.
 */
class Channel {
 public:
  // The other end of the Channel.
  virtual uni::net::endpoint_id endpoint_id() = 0;

  // Queues a message to be send to the other end of the channel.
  virtual void queue_send(std::string message) {};

  virtual void start_listening() {};

  // The callback should return true if the Channel should continue listening
  // messages. Returning false make the stop listening to receiving data.
  virtual void set_receive_callback(std::function<bool(std::string)> callback) {};

  virtual void set_close_callback(std::function<void(void)> callback) {};
};

} // namespace net
} // namespace uni


#endif // UNI_NET_CHANNEL_H
