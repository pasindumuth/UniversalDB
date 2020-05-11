#ifndef UNI_NET_SELFCHANNEL_H
#define UNI_NET_SELFCHANNEL_H

#include <functional>
#include <string>
#include <vector>

#include <common/common.h>
#include <net/EndpointId.h>
#include <net/Channel.h>

namespace uni {
namespace net {

class SelfChannel
  : public uni::net::Channel {
 public:
  SelfChannel(std::string ip_string);

  ~SelfChannel() {};

  uni::net::EndpointId endpoint_id() override;

  void queue_send(std::string message) override;

  void start_listening() override;

 private:
  // Indicates if start_listening was called.
  bool listening;
  std::string _ip_string;
};

} // namespace net
} // namespace uni


#endif // UNI_NET_SELFCHANNEL_H
