#ifndef UNI_SLAVE_TESTINGCONTEXT_H
#define UNI_SLAVE_TESTINGCONTEXT_H

#include <functional>
#include <memory>
#include <thread>
#include <vector>

#include <async/AsyncQueue.h>
#include <async/testing/AsyncSchedulerTesting.h>
#include <async/testing/ClockTesting.h>
#include <async/testing/TimerAsyncSchedulerTesting.h>
#include <constants/constants.h>
#include <common/common.h>
#include <net/Connections.h>
#include <net/EndpointId.h>
#include <paxos/PaxosLog.h>
#include <random/testing/RandomTesting.h>
#include <server/FailureDetector.h>
#include <server/HeartbeatTracker.h>
#include <server/LogSyncer.h>
#include <slave/TabletParticipant.h>
#include <slave/SlaveConfigManager.h>
#include <slave/SlaveIncomingMessageHandler.h>
#include <slave/SlaveKeySpaceManager.h>

namespace uni {
namespace slave {

struct TestingContext {
  TestingContext(
    uni::constants::Constants const& constants,
    std::string ip_string,
    unsigned random_seed);

  // Primitives
  std::string _ip_string;

  // Helper classes
  uni::random::RandomTesting _random;

  // Singletons
  uni::async::AsyncSchedulerTesting _scheduler;
  uni::net::Connections _client_connections;
  uni::net::Connections _master_connections;
  uni::net::Connections _connections;
  uni::async::ClockTesting _clock;
  uni::async::TimerAsyncSchedulerTesting _timer_scheduler;
  uni::server::HeartbeatTracker _heartbeat_tracker;
  uni::server::FailureDetector _failure_detector;
  uni::paxos::PaxosLog _paxos_log;
  uni::async::AsyncQueue _async_queue;
  uni::paxos::MultiPaxosHandler _multipaxos_handler;
  uni::server::LogSyncer _log_syncer;
  uni::slave::SlaveConfigManager _config_manager;
  uni::slave::SlaveKeySpaceManager _key_space_manager;
  uni::slave::SlaveIncomingMessageHandler _slave_handler;
};

} // namespace slave
} // namespace uni

#endif // UNI_SLAVE_TESTINGCONTEXT_H
