#ifndef UNI_SLAVE_FAILUREDETECTOR
#define UNI_SLAVE_FAILUREDETECTOR

#include <map>

#include <net/endpoint_id.h>
#include <proto/slave.pb.h>

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
  FailureDetector();

  void handle_heartbeat(uni::net::endpoint_id endpoint_id);

 private:
  std::map<uni::net::endpoint_id, unsigned> heartbeat_count;
};

} // namespace slave
} // namespace uni

#endif // UNI_SLAVE_FAILUREDETECTOR
