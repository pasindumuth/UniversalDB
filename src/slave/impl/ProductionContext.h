#ifndef UNI_SLAVE_PRODUCTIONCONTEXT_H
#define UNI_SLAVE_PRODUCTIONCONTEXT_H

#include <functional>
#include <memory>
#include <thread>
#include <vector>

#include <boost/asio.hpp>

#include <async/AsyncQueue.h>
#include <async/impl/AsyncSchedulerImpl.h>
#include <async/impl/TimerAsyncSchedulerImpl.h>
#include <constants/constants.h>
#include <common/common.h>
#include <net/Connections.h>
#include <net/EndpointId.h>
#include <paxos/PaxosLog.h>
#include <random/impl/RandomImpl.h>
#include <server/FailureDetector.h>
#include <server/HeartbeatTracker.h>
#include <server/LogSyncer.h>
#include <slave/SlaveConfigManager.h>
#include <slave/SlaveIncomingMessageHandler.h>
#include <slave/TabletParticipantManager.h>
#include <slave/SlaveKeySpaceManager.h>

namespace uni {
namespace slave {

struct ProductionContext {
  struct ThreadAndContext {
    boost::asio::io_context _io_context;
    std::thread _thread;

    ThreadAndContext();
  };

  ProductionContext(
    boost::asio::io_context& background_io_context,
    uni::constants::Constants const& constants,
    uni::net::Connections& client_connections,
    uni::net::Connections& master_connections,
    uni::net::Connections& connections,
    uni::async::AsyncSchedulerImpl& scheduler,
    std::string ip_string);

  // Helper classes
  uni::random::RandomImpl _random;

  // Singletons
  uni::async::TimerAsyncSchedulerImpl _timer_scheduler;
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

 private:
  std::vector<unsigned> _participants_per_thread;
  std::vector<std::unique_ptr<ThreadAndContext>> _io_contexts;
};

} // namespace slave
} // namespace uni

#endif // UNI_SLAVE_PRODUCTIONCONTEXT_H
