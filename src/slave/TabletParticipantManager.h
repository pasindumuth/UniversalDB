#ifndef UNI_SLAVE_TABLETPARTICIPANTMANAGER_H
#define UNI_SLAVE_TABLETPARTICIPANTMANAGER_H

#include <memory>
#include <thread>
#include <unordered_map>
#include <vector>

#include <async/AsyncScheduler.h>
#include <async/TimerAsyncScheduler.h>
#include <common/common.h>
#include <slave/FailureDetector.h>
#include <slave/LogSyncer.h>
#include <slave/HeartbeatTracker.h>
#include <slave/TabletId.h>
#include <slave/TabletParticipant.h>
#include <slave/ThreadPool.h>

namespace uni {
namespace slave {

class TabletParticipantManager {
 public:
  TabletParticipantManager(
    uni::slave::ThreadPool& thread_pool,
    uni::net::ConnectionsIn& client_connections_in,
    uni::net::ConnectionsOut& connections_out,
    uni::slave::FailureDetector& failure_detector,
    uni::slave::HeartbeatTracker& heartbeat_tracker,
    uni::slave::LogSyncer& log_syncer,
    uni::constants::Constants const& constants,
    uni::async::TimerAsyncScheduler& timer_scheduler);

  std::unique_ptr<uni::slave::TabletParticipant>& getTablet(TabletId tablet_id);

 private:
  uni::slave::ThreadPool& _thread_pool;
  uni::net::ConnectionsIn& _client_connections_in;
  uni::net::ConnectionsOut& _connections_out;
  uni::slave::FailureDetector& _failure_detector;
  uni::slave::HeartbeatTracker& _heartbeat_tracker;
  uni::slave::LogSyncer& _log_syncer;
  uni::constants::Constants const& _constants;
  uni::async::TimerAsyncScheduler& _timer_scheduler;

  std::vector<unsigned> _participants_per_thread;
  std::unordered_map<TabletId, std::unique_ptr<uni::slave::TabletParticipant>> _tp_map;
};

} // namespace slave
} // namespace uni

#endif // UNI_SLAVE_TABLETPARTICIPANTMANAGER_H
