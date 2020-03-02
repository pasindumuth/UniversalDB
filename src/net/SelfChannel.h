#ifndef UNI_NET_SELFCHANNEL_H
#define UNI_NET_SELFCHANNEL_H

#include <functional>
#include <string>
#include <vector>

#include <common/common.h>
#include <net/endpoint_id.h>
#include <net/Channel.h>

namespace uni {
namespace net {

class SelfChannel
  : public uni::net::Channel {
 public:
  SelfChannel();

  ~SelfChannel() {};

  uni::net::endpoint_id endpoint_id() override;

  void queue_send(std::string message) override;

  void start_listening() override;

 private:
  // Indicates if start_listening was called.
  bool listening;
};

} // namespace net
} // namespace uni


#endif // UNI_NET_SELFCHANNEL_H
