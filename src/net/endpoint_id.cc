#include "endpoint_id.h"

#include <boost/functional/hash.hpp>

namespace uni {
namespace net {

endpoint_id::endpoint_id(std::string const& ip_string, int32_t const& port)
  : ip_string(ip_string),
    port(port) {}

bool endpoint_id::operator==(const endpoint_id& other) const {
  return ip_string == other.ip_string && port == other.port;
}

bool endpoint_id::operator<(const endpoint_id& other) const {
  return ip_string < other.ip_string;
}

std::size_t endpoint_id::hash() const {
  // Initial hash value
  std::size_t seed = 0;

  // Combine each private member into the hash value
  boost::hash_combine(seed, boost::hash_value(ip_string));
  boost::hash_combine(seed, boost::hash_value(port));

  return seed;
}

} // namespace net
} // namespace uni
