#ifndef UNI_NET_ENDPOINTID
#define UNI_NET_ENDPOINTID

#include <string>

namespace uni {
namespace net {

struct endpoint_id {
  std::string const ip_string;
  int const port;

  endpoint_id(std::string const& ip_string, int const& port);

  bool operator==(const endpoint_id& other) const;

  bool operator<(const endpoint_id& other) const;

  std::size_t hash() const;
};

} // namespace net
} // namespace uni


namespace std {

template<>
struct hash<uni::net::endpoint_id> {
  std::size_t operator()(const uni::net::endpoint_id& id) const {
    return id.hash();
  }
};

} // namespace std

#endif //UNI_NET_ENDPOINTID
