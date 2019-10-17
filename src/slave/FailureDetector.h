#ifndef UNI_SLAVE_FAILUREDETECTOR
#define UNI_SLAVE_FAILUREDETECTOR

#include <map>
#include <vector>

#include <constants/constants.h>
#include <net/ConnectionsOut.h>
#include <net/endpoint_id.h>
#include <proto/message.pb.h>
#include <async/TimerAsyncScheduler.h>

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

  // Handle an incoming heartbeat message. Since we only care about whot the
  // hearbeat came from, we don't actually pass the heartbest protobuf
  // message into this function.
  void handle_heartbeat(uni::net::endpoint_id endpoint_id);

  // Get a list of endpoints that are still alive (endpoints that haven't failed).
  std::vector<uni::net::endpoint_id> alive_endpoints();

 private:
  std::map<uni::net::endpoint_id, unsigned> _heartbeat_count;
  uni::constants::Constants const& _constants;
  uni::net::ConnectionsOut& _connections_out;
  uni::async::TimerAsyncScheduler& _timer_scheduler;
  // Heartbeat message used across all sends
  proto::message::MessageWrapper _message;
};

} // namespace slave
} // namespace uni

#endif // UNI_SLAVE_FAILUREDETECTOR
