#ifndef UNI_SLAVE_FAILUREDETECTOR
#define UNI_SLAVE_FAILUREDETECTOR

#include <map>
#include <vector>

#include <boost/optional.hpp>

#include <async/TimerAsyncScheduler.h>
#include <common/common.h>
#include <constants/constants.h>
#include <net/ConnectionsOut.h>
#include <net/endpoint_id.h>
#include <proto/message.pb.h>

namespace uni {
namespace slave {

/**
 * This class maintains a list of Paxos participants that have not failed. The
 * technique we use to detect liveness is heartbeats. Each node sends all other
 * nodes a heartbeat in some predefined period. If a heartbeat isn't detected
 * after some number of periods, then that node is marked as failed. Note that
 * this is a weak failure detector; a node that is marked as failed need not
 * have actually failed. Additionally, heartbeats from a node can in principle
 * be arbitrarily delayed, which means that some node might send a heartbeat, then
 * fail. If that heartbeat is received by other nodes a minute later, those
 * nodes will mark the failed as alive. This is not a bug in the failure detector,
 * and any algorithm that uses this failure detector must accept such situations.
 */
class FailureDetector {
 public:
  FailureDetector(
    uni::constants::Constants const& constants,
    uni::net::ConnectionsOut& connections_out,
    uni::async::TimerAsyncScheduler& timer_scheduler);

  // Schedule heartbeat to be sent periodically
  void schedule_heartbeat();

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

 private:
  std::map<uni::net::endpoint_id, uint32_t> _heartbeat_count;
  uni::constants::Constants const& _constants;
  uni::net::ConnectionsOut& _connections_out;
  uni::async::TimerAsyncScheduler& _timer_scheduler;
  // Heartbeat message used across all sends
  proto::message::MessageWrapper _message;

  void debug_print();
};

} // namespace slave
} // namespace uni

#endif // UNI_SLAVE_FAILUREDETECTOR
