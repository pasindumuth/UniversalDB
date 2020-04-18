#ifndef UNI_MASTER_TESTINGCONTEXT_H
#define UNI_MASTER_TESTINGCONTEXT_H

#include <functional>
#include <string>

#include <async/AsyncQueue.h>
#include <async/testing/AsyncSchedulerTesting.h>
#include <async/testing/ClockTesting.h>
#include <async/testing/TimerAsyncSchedulerTesting.h>
#include <constants/constants.h>
#include <common/common.h>
#include <master/GroupConfigManager.h>
#include <master/KeySpaceManager.h>
#include <master/MasterIncomingMessageHandler.h>
#include <net/Connections.h>
#include <net/EndpointId.h>
#include <paxos/MultiPaxosHandler.h>
#include <paxos/PaxosLog.h>
#include <random/testing/RandomTesting.h>
#include <server/LogSyncer.h>

namespace uni {
namespace master {

struct TestingContext {
  TestingContext(
    uni::constants::Constants const& constants,
    std::vector<uni::net::EndpointId>& config_endpoints,
    std::string ip_string,
    unsigned random_seed);

  // Primitives
  std::string _ip_string;

  // Helper classes
  uni::random::RandomTesting _random;

  // Providers
  std::function<uni::async::AsyncQueue()> _async_queue_provider;

  // Singletons
  uni::async::AsyncSchedulerTesting _scheduler;
  uni::net::Connections _client_connections;
  uni::net::Connections _slave_connections;
  uni::net::Connections _connections;
  uni::async::ClockTesting _clock;
  uni::async::TimerAsyncSchedulerTesting _timer_scheduler;
  uni::paxos::PaxosLog _paxos_log;
  uni::async::AsyncQueue _async_queue;
  uni::paxos::MultiPaxosHandler _multipaxos_handler;
  uni::server::LogSyncer _log_syncer;
  uni::master::GroupConfigManager _group_config_manager;
  uni::master::KeySpaceManager _key_space_manager;
  uni::master::MasterIncomingMessageHandler _master_handler;
};

} // namespace master
} // namespace uni

#endif // UNI_MASTER_TESTINGCONTEXT_H
