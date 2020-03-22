#ifndef UNI_NET_ENDPOINTID
#define UNI_NET_ENDPOINTID

#include <string>

#include <common/common.h>

namespace uni {
namespace net {

struct EndpointId {
  std::string ip_string;
  int32_t port;

  EndpointId(std::string const& ip_string, int32_t const& port);

  bool operator==(const EndpointId& other) const;

  bool operator<(const EndpointId& other) const;

  std::size_t hash() const;
};

} // namespace net
} // namespace uni


namespace std {

template<>
struct hash<uni::net::EndpointId> {
  std::size_t operator()(const uni::net::EndpointId& id) const {
    return id.hash();
  }
};

} // namespace std

#endif //UNI_NET_ENDPOINTID
