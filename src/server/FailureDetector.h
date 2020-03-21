#ifndef UNI_SERVER_FAILUREDETECTOR
#define UNI_SERVER_FAILUREDETECTOR

#include <async/TimerAsyncScheduler.h>
#include <common/common.h>
#include <net/Connections.h>
#include <proto/message.pb.h>
#include <server/HeartbeatTracker.h>

namespace uni {
namespace server {

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
    uni::server::HeartbeatTracker& heartbeat_tracker,
    uni::net::Connections& connections,
    uni::async::TimerAsyncScheduler& timer_scheduler);

  // Schedule heartbeat to be sent periodically
  void schedule_heartbeat();

 private:
  uni::server::HeartbeatTracker& _heartbeat_tracker;
  uni::net::Connections& _connections;
  uni::async::TimerAsyncScheduler& _timer_scheduler;
  // Heartbeat message used across all sends
  proto::message::MessageWrapper _message;
};

} // namespace server
} // namespace uni

#endif // UNI_SERVER_FAILUREDETECTOR
