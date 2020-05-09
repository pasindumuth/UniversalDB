#ifndef UNI_SLAVE_LOCALTRANSACTIONMANAGER_H
#define UNI_SLAVE_LOCALTRANSACTIONMANAGER_H

#include <functional>
#include <memory>
#include <string>

#include <async/AsyncQueue.h>
#include <async/TimerAsyncScheduler.h>
#include <constants/constants.h>
#include <common/common.h>
#include <net/Connections.h>
#include <paxos/PaxosLog.h>
#include <random/Random.h>
#include <server/FailureDetector.h>
#include <server/HeartbeatTracker.h>
#include <server/LogSyncer.h>
#include <slave/TabletId.h>
#include <slave/TabletParticipant.h>
#include <slave/SlaveConfigManager.h>
#include <slave/SlaveIncomingMessageHandler.h>
#include <slave/SlaveKeySpaceManager.h>

namespace uni {
namespace slave {

struct LocalTransactionManager {
  using TPProvider = std::function<uni::custom_unique_ptr<uni::slave::TabletParticipant>(
    uni::slave::TabletId const&,
    uni::server::FailureDetector&,
    uni::slave::SlaveConfigManager&)>;

  LocalTransactionManager(
    uni::constants::Constants const& constants,
    uni::net::Connections& client_connections,
    uni::net::Connections& master_connections,
    uni::net::Connections& slave_connections,
    uni::async::AsyncScheduler& scheduler,
    uni::async::TimerAsyncScheduler& timer_scheduler,
    uni::random::Random& random,
    std::string& ip_string,
    TPProvider tp_provider);

  // Singletons
  uni::async::AsyncQueue _async_queue;
  uni::server::HeartbeatTracker _heartbeat_tracker;
  uni::server::FailureDetector _failure_detector;
  uni::paxos::PaxosLog _paxos_log;
  uni::paxos::MultiPaxosHandler _multipaxos_handler;
  uni::server::LogSyncer _log_syncer;
  uni::slave::SlaveConfigManager _config_manager;
  uni::slave::SlaveKeySpaceManager _key_space_manager;
  uni::slave::TabletParticipantManager _tablet_manager;
  uni::slave::SlaveIncomingMessageHandler _slave_handler;
};

} // namespace slave
} // namespace uni

#endif // UNI_SLAVE_LOCALTRANSACTIONMANAGER_H
