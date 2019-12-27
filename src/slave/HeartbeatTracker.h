#ifndef UNI_SLAVE_HEARTBEATTRACKER
#define UNI_SLAVE_HEARTBEATTRACKER

#include <map>
#include <vector>

#include <boost/optional.hpp>

#include <common/common.h>
#include <net/endpoint_id.h>

namespace uni {
namespace slave {

class HeartbeatTracker {
 public:
  static uint32_t const HEARTBEAT_FAILURE_COUNT = 4; // The number of heartbeat cycles that must pass before a node is marked as failed.
  static uint32_t const HEARTBEAT_SEND_PERIOD = 1000; // The period in which heartbeats are sent in milliseconds.

  HeartbeatTracker();

  HeartbeatTracker(std::map<uni::net::endpoint_id, uint32_t> heartbeat_count);

  void increment_counts();

  // Handle an incoming heartbeat message. Since we only care about who the
  // hearbeat came from, we don't actually pass the heartbest protobuf
  // message into this function.
  void handle_heartbeat(uni::net::endpoint_id endpoint_id);

  // Get an ordered list of endpoints that are still alive (endpoints that haven't failed).
  std::vector<uni::net::endpoint_id> alive_endpoints();

  // Gets the endpoint_id of the leader. The leader is defined the node with the smallest
  // endpoint_id (endpoints_ids are totally ordered) which is not detected as dead. Recall
  // that a node's perceived leader doesn't need to be consistent across all nodes. However
  // as the network stabilizes and after some after a node's failure, all nodes will end up
  // having the same perceived leader, which is what's important.
  // 
  // We return an optional in case there is a moment, for whatever reason, a leader couldn't
  // be found. This should never happen in practice because this node would send itself a heartbeat
  // message. But if for whatever reason that doesn't happen, the alive_endpoints() list would
  // be empty, and there would be no leader.
  boost::optional<uni::net::endpoint_id> leader_endpoint_id();

  void debug_print();

  private:
    std::map<uni::net::endpoint_id, uint32_t> _heartbeat_count;
};

} // namespace slave
} // namespace uni

#endif // UNI_SLAVE_HEARTBEATTRACKER
