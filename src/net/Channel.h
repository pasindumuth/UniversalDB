#ifndef UNI_NET_CHANNEL_H
#define UNI_NET_CHANNEL_H

#include <functional>
#include <string>

#include <common/common.h>
#include <net/endpoint_id.h>

namespace uni {
namespace net {

/**
 * This class provides a simple interface to the network. We use the
 * Semi-FIFO Network model. This class represents an interface for both the
 * In-Semi-FIFO Channel as well as the Out-Semi-FIFO Channel.
 */
class Channel {
 public:
  virtual ~Channel() {};

  // The other end of the Channel.
  virtual uni::net::endpoint_id endpoint_id() = 0;

  // Queues a message to be send to the other end of the channel.
  virtual void queue_send(std::string message) {};

  virtual void start_listening() {};

  /**
   * Registers a callback to run when data is received from the other endpoint.
   * The callback returns a boolean indicating whether to continue running that callback
   * when data is received; i.e. if false is returned, then the callback stops being run.
   */
  virtual void add_receive_callback(std::function<bool(std::string)> callback) {};

  virtual void add_close_callback(std::function<void(void)> callback) {};
};

} // namespace net
} // namespace uni


#endif // UNI_NET_CHANNEL_H
