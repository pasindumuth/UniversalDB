#include "EndpointId.h"

#include <boost/functional/hash.hpp>

namespace uni {
namespace net {

EndpointId::EndpointId(std::string const& ip_string, int32_t const& port)
  : ip_string(ip_string),
    port(port) {}

bool EndpointId::operator==(const EndpointId& other) const {
  return ip_string == other.ip_string && port == other.port;
}

bool EndpointId::operator<(const EndpointId& other) const {
  return ip_string < other.ip_string;
}

std::size_t EndpointId::hash() const {
  // Initial hash value
  std::size_t seed = 0;

  // Combine each private member into the hash value
  boost::hash_combine(seed, boost::hash_value(ip_string));
  boost::hash_combine(seed, boost::hash_value(port));

  return seed;
}

} // namespace net
} // namespace uni
