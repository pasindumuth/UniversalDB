#ifndef UNI_CONSTANTS_CONSTANTS_H
#define UNI_CONSTANTS_CONSTANTS_H

#include <common/common.h>

namespace uni {
namespace constants {

struct Constants {
  uint32_t const num_slave_servers;
  uint32_t const slave_port; // Slaves communicates with slaves through this port.
  uint32_t const client_port; // Client communicates with slaves through this port.
  uint32_t const master_port; // Master communicates with slaves through this port.
  int64_t const heartbeat_period; // The period in which heartbeats are sent in milliseconds.
  uint32_t const heartbeat_failure_threshold; // The number of heartbeat cycles that must pass before a node is marked as failed.
  int64_t const log_syncer_period; // The period in which the Syncer syncs the PaxosLog with the leader's PaxosLog

  Constants(
      uint32_t const& num_slave_servers,
      uint32_t const& slave_port,
      uint32_t const& client_port,
      uint32_t const& master_port,
      int64_t const& heartbeat_period,
      uint32_t const heartbeat_failure_threshold,
      int64_t const log_syncer_period)
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
