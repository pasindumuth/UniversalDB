#ifndef UNI_MASTER_PRODUCTIONCONTEXT_H
#define UNI_MASTER_PRODUCTIONCONTEXT_H

#include <functional>
#include <memory>
#include <thread>
#include <vector>

#include <boost/asio.hpp>

#include <async/impl/TimerAsyncSchedulerImpl.h>
#include <async/AsyncQueue.h>
#include <constants/constants.h>
#include <common/common.h>
#include <master/GroupConfigManager.h>
#include <master/KeySpaceManager.h>
#include <master/MasterIncomingMessageHandler.h>
#include <net/Connections.h>
#include <paxos/PaxosLog.h>
#include <paxos/MultiPaxosHandler.h>
#include <server/FailureDetector.h>
#include <server/HeartbeatTracker.h>
#include <server/LogSyncer.h>

namespace uni {
namespace master {

struct ProductionContext {
  ProductionContext(
    boost::asio::io_context& background_io_context,
    uni::constants::Constants const& constants,
    uni::net::Connections& client_connections,
    uni::net::Connections& slave_connections,
    uni::net::Connections& connections);

  uni::async::TimerAsyncSchedulerImpl timer_scheduler;
  uni::async::AsyncQueue async_queue;
  uni::paxos::PaxosLog paxos_log;
  uni::paxos::MultiPaxosHandler multipaxos_handler;
  uni::server::LogSyncer log_syncer;
  uni::master::GroupConfigManager group_config_manager;
  uni::master::KeySpaceManager key_space_manager;
  uni::master::MasterIncomingMessageHandler master_handler;
};

} // namespace master
} // namespace uni

#endif // UNI_MASTER_PRODUCTIONCONTEXT_H
