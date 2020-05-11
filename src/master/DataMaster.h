#ifndef UNI_MASTER_DATAMASTER_H
#define UNI_MASTER_DATAMASTER_H

#include <functional>

#include <async/AsyncQueue.h>
#include <async/AsyncScheduler.h>
#include <async/TimerAsyncScheduler.h>
#include <constants/constants.h>
#include <common/common.h>
#include <master/GroupConfigManager.h>
#include <master/KeySpaceManager.h>
#include <master/MasterIncomingMessageHandler.h>
#include <net/Connections.h>
#include <paxos/PaxosConfigManager.h>
#include <paxos/PaxosLog.h>
#include <paxos/MultiPaxosHandler.h>
#include <random/Random.h>
#include <server/LogSyncer.h>

namespace uni {
namespace master {

struct DataMaster {
  DataMaster(
    uni::constants::Constants const& constants,
    uni::net::Connections& client_connections,
    uni::net::Connections& slave_connections,
    uni::net::Connections& connections,
    uni::async::TimerAsyncScheduler& timer_scheduler,
    uni::async::AsyncScheduler& scheduler,
    uni::random::Random& random,
    std::vector<uni::net::EndpointId> slave_endpoints,
    std::vector<uni::net::EndpointId> master_endpoints);

  // Providers
  std::function<uni::async::AsyncQueue()> _async_queue_provider;

  // Singletons
  uni::async::AsyncQueue _async_queue;
  uni::paxos::PaxosLog _paxos_log;
  uni::paxos::PaxosConfigManager _paxos_config_manager;
  uni::paxos::MultiPaxosHandler _multipaxos_handler;
  uni::server::LogSyncer _log_syncer;
  uni::master::GroupConfigManager _group_config_manager;
  uni::master::KeySpaceManager _key_space_manager;
  uni::master::MasterIncomingMessageHandler _master_handler;
};

} // namespace master
} // namespace uni

#endif // UNI_MASTER_DATAMASTER_H
