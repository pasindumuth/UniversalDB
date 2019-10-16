#ifndef UNI_CONSTANTS_CONSTANTS_H
#define UNI_CONSTANTS_CONSTANTS_H

namespace uni {
namespace constants {

struct Constants {
  unsigned const num_slave_servers;
  unsigned const slave_port; // Slaves communicates with slaves through this port.
  unsigned const client_port; // Client communicates with slaves through this port.
  unsigned const master_port; // Master communicates with slaves through this port.
  unsigned const heartbeat_wait_ms; // The period in which heartbeats are sent in milliseconds.

  Constants(
      unsigned const& num_slave_servers,
      unsigned const& slave_port,
      unsigned const& client_port,
      unsigned const& master_port,
      unsigned const& heartbeat_wait_ms)
        : num_slave_servers(num_slave_servers),
          slave_port(slave_port),
          client_port(client_port),
          master_port(master_port),
          heartbeat_wait_ms(heartbeat_wait_ms) {}
};

} // namespace constants
} // namespace uni


#endif // UNI_CONSTANTS_CONSTANTS_H
