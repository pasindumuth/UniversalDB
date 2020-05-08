#ifndef UNI_SLAVE_TABLETPARTICIPANT_H
#define UNI_SLAVE_TABLETPARTICIPANT_H

#include <functional>
#include <memory>
#include <thread>

#include <boost/asio.hpp>

#include <async/impl/AsyncSchedulerImpl.h>
#include <async/AsyncScheduler.h>
#include <common/common.h>
#include <constants/constants.h>
#include <net/Connections.h>
#include <paxos/MultiPaxosHandler.h>
#include <paxos/PaxosLog.h>
#include <random/Random.h>
#include <slave/ClientRequestHandler.h>
#include <slave/IncomingMessageHandler.h>
#include <slave/SlaveConfigManager.h>
#include <slave/KVStore.h>
#include <server/LogSyncer.h>
#include <async/AsyncQueue.h>
#include <slave/TabletId.h>

namespace uni {
namespace slave {

struct TabletParticipant {
  TabletParticipant(
    std::function<std::unique_ptr<uni::async::AsyncScheduler>()> scheduler_provider,
    std::unique_ptr<uni::random::Random> random,
    uni::constants::Constants const& constants,
    uni::net::Connections& slave_connections,
    uni::net::Connections& client_connections,
    uni::async::TimerAsyncScheduler& timer_scheduler,
    uni::server::FailureDetector& failure_detector,
    uni::slave::SlaveConfigManager& config_manager,
    uni::slave::TabletId& tid);

  // Helper classes
  std::unique_ptr<uni::random::Random> _random;

  // Singletons
  std::unique_ptr<uni::async::AsyncScheduler> _scheduler;
  uni::slave::TabletId _tablet_id;
  uni::paxos::PaxosLog _paxos_log;
  uni::paxos::MultiPaxosHandler _multipaxos_handler;
  uni::async::AsyncQueue _async_queue;
  uni::slave::KVStore _kvstore;
  uni::slave::ClientRequestHandler _client_request_handler;
  uni::server::LogSyncer _log_syncer;
  uni::slave::IncomingMessageHandler _incoming_message_handler;
};

} // namespace slave
} // namespace uni

#endif // UNI_SLAVE_TABLETPARTICIPANT_H
