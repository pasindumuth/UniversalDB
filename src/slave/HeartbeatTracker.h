#ifndef UNI_SLAVE_HEARTBEATTRACKER
#define UNI_SLAVE_HEARTBEATTRACKER

#include <map>
#include <vector>
#include <sstream>

#include <boost/optional.hpp>

#include <common/common.h>
#include <net/endpoint_id.h>

namespace uni {
namespace slave {

class HeartbeatTracker {
 public:
  static uint32_t const HEARTBEAT_FAILURE_COUNT; // The number of heartbeat cycles that must pass before a node is marked as failed.
  static uint32_t const HEARTBEAT_SEND_PERIOD; // The period in which heartbeats are sent in milliseconds.

  HeartbeatTracker();

  // Every Heartbeat send period, we send out a heartbeat, but we also
  // increment the time since the last heartbeat for all endpoints that
  // are currently considered alive. This method doesn' trim off newly dead nodes.
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

  // Creates a debug string representing this class.
  std::string debug_string();

  private:
    std::map<uni::net::endpoint_id, uint32_t> _heartbeat_count;
};

namespace _inner {

void increment_counts(std::map<uni::net::endpoint_id, uint32_t>& heartbeat_count);
void handle_heartbeat(std::map<uni::net::endpoint_id, uint32_t>& heartbeat_count, uni::net::endpoint_id endpoint_id);
std::vector<uni::net::endpoint_id> alive_endpoints(std::map<uni::net::endpoint_id, uint32_t>& heartbeat_count);
boost::optional<uni::net::endpoint_id> leader_endpoint_id(std::map<uni::net::endpoint_id, uint32_t>& heartbeat_count);
std::string debug_string(std::map<uni::net::endpoint_id, uint32_t>& heartbeat_count);

} // namespace _inner
} // namespace slave
} // namespace uni

#endif // UNI_SLAVE_HEARTBEATTRACKER
