#ifndef UNI_NET_INCOMINGMESSAGE_H
#define UNI_NET_INCOMINGMESSAGE_H

#include <string>

#include <common/common.h>
#include <net/EndpointId.h>

namespace uni {
namespace net {

struct IncomingMessage {
  uni::net::EndpointId endpoint_id;
  std::string message;

  IncomingMessage(uni::net::EndpointId endpoint_id, std::string message)
      : endpoint_id(endpoint_id),
        message(message) {}
};

} // net
} // uni


#endif // UNI_NET_INCOMINGMESSAGE_H
