#ifndef UNI_SERVER_LOGSYNCER
#define UNI_SERVER_LOGSYNCER

#include <vector>
#include <functional>

#include <async/TimerAsyncScheduler.h>
#include <common/common.h>
#include <constants/constants.h>
#include <net/Connections.h>
#include <net/endpoint_id.h>
#include <paxos/PaxosLog.h>
#include <paxos/PaxosTypes.h>
#include <proto/message.pb.h>
#include <proto/slave.pb.h>
#include <proto/sync.pb.h>
#include <server/FailureDetector.h>


namespace uni {
namespace server {

class LogSyncer {
 public:
  LogSyncer(
    uni::constants::Constants const& constants,
    uni::net::Connections& connections,
    uni::async::TimerAsyncScheduler& timer_scheduler,
    uni::paxos::PaxosLog& paxos_log,
    std::function<proto::message::MessageWrapper(proto::sync::SyncMessage*)> sync_message_to_wrapper);

  void schedule_syncing();

  void handle_sync_request(uni::net::endpoint_id endpoint_id, proto::sync::SyncRequest request);

  void handle_sync_response(proto::sync::SyncResponse response);

 private:
  uni::constants::Constants const& _constants;
  uni::net::Connections& _connections;
  uni::async::TimerAsyncScheduler& _timer_scheduler;
  uni::paxos::PaxosLog& _paxos_log;
  std::function<proto::message::MessageWrapper(proto::sync::SyncMessage*)> _sync_message_to_wrapper;
};

namespace _inner {

proto::sync::SyncRequest* build_sync_request(std::vector<uni::paxos::index_t> available_indices);
proto::sync::SyncResponse* build_sync_response(uni::paxos::PaxosLog& paxos_log, proto::sync::SyncRequest& request);

} // namespace _inner
} // namespace server
} // namespace uni

#endif // UNI_SERVER_LOGSYNCER
