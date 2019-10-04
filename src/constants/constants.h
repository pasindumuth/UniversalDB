#ifndef UNI_CONSTANTS_CONSTANTS_H
#define UNI_CONSTANTS_CONSTANTS_H

namespace uni {
namespace constants {

struct Constants {
  unsigned const num_slave_servers;
  unsigned const slave_port;

  Constants(
      unsigned const& num_slave_servers,
      unsigned const& slave_port)
        : num_slave_servers(num_slave_servers),
          slave_port(slave_port) {}
};

} // namespace constants
} // namespace uni


#endif // UNI_CONSTANTS_CONSTANTS_H
