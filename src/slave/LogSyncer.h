#ifndef UNI_SLAVE_LOGSYNCER
#define UNI_SLAVE_LOGSYNCER

#include <async/TimerAsyncScheduler.h>
#include <constants/constants.h>
#include <net/ConnectionsOut.h>
#include <net/endpoint_id.h>
#include <paxos/PaxosLog.h>
#include <proto/message.pb.h>
#include <proto/slave.pb.h>
#include <slave/FailureDetector.h>

namespace uni {
namespace slave {

class LogSyncer {
 public:
  LogSyncer(
    uni::constants::Constants const& constants,
    uni::net::ConnectionsOut& connections_out,
    uni::async::TimerAsyncScheduler& timer_scheduler,
    uni::paxos::PaxosLog& paxos_log,
    uni::slave::FailureDetector& failure_detector);

  void schedule_syncing();

  void handle_sync_request(uni::net::endpoint_id endpoint_id, proto::slave::SyncRequest request);

  void handle_sync_response(proto::slave::SyncResponse response);

 private:
  uni::constants::Constants const& _constants;
  uni::net::ConnectionsOut& _connections_out;
  uni::async::TimerAsyncScheduler& _timer_scheduler;
  uni::paxos::PaxosLog& _paxos_log;
  uni::slave::FailureDetector _failure_detector;
};

} // namespace slave
} // namespace uni

#endif // UNI_SLAVE_LOGSYNCER