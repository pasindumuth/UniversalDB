#ifndef UNI_NET_CHANNEL_H
#define UNI_NET_CHANNEL_H

#include <functional>
#include <string>

#include <net/endpoint_id.h>

namespace uni {
namespace net {

class Channel {
 public:
  virtual uni::net::endpoint_id endpoint_id() = 0;

  virtual void queue_send(std::string message) {};

  virtual void start_listening() {};

  virtual void set_recieve_callback(std::function<void(std::string)> callback) {};

  virtual void set_close_callback(std::function<void(void)> callback) {};
};

} // namespace net
} // namespace uni


#endif // UNI_NET_CHANNEL_H
