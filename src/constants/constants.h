#ifndef UNI_CONSTANTS_CONSTANTS_H
#define UNI_CONSTANTS_CONSTANTS_H

namespace uni {
namespace constants {

struct Constants {
  unsigned const num_slave_servers;
  unsigned const slave_port; // Slaves communicates with slaves through this port.
  unsigned const client_port; // Client communicates with slaves through this port.
  unsigned const master_port; // Master communicates with slaves through this port.
  long const heartbeat_period; // The period in which heartbeats are sent in milliseconds.
  unsigned const heartbeat_failure_threshold; // The number of heartbeat cycles that must pass before a node is marked as failed.
  long const log_syncer_period; // The period in which the Syncer syncs the PaxosLog with the leader's PaxosLog

  Constants(
      unsigned const& num_slave_servers,
      unsigned const& slave_port,
      unsigned const& client_port,
      unsigned const& master_port,
      long const& heartbeat_period,
      unsigned const heartbeat_failure_threshold,
      long const log_syncer_period)
        : num_slave_servers(num_slave_servers),
          slave_port(slave_port),
          client_port(client_port),
          master_port(master_port),
          heartbeat_period(heartbeat_period),
          heartbeat_failure_threshold(heartbeat_failure_threshold),
          log_syncer_period(log_syncer_period) {}
};

} // namespace constants
} // namespace uni


#endif // UNI_CONSTANTS_CONSTANTS_H
