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
#include <server/LogSyncer.h>

namespace uni {
namespace master {

struct TestingContext {
  TestingContext(
    uni::constants::Constants const& constants,
    std::vector<uni::net::EndpointId>& config_endpoints,
    std::string ip_string);

  // Providers
  std::function<uni::async::AsyncQueue()> async_queue_provider;

  // Singletons
  std::string ip_string;
  uni::async::AsyncSchedulerTesting scheduler;
  uni::net::Connections client_connections;
  uni::net::Connections slave_connections;
  uni::net::Connections connections;
  uni::async::ClockTesting clock;
  uni::async::TimerAsyncSchedulerTesting timer_scheduler;
  uni::paxos::PaxosLog paxos_log;
  uni::async::AsyncQueue async_queue;
  uni::paxos::MultiPaxosHandler multipaxos_handler;
  uni::server::LogSyncer log_syncer;
  uni::master::GroupConfigManager group_config_manager;
  uni::master::KeySpaceManager key_space_manager;
  uni::master::MasterIncomingMessageHandler master_handler;
};

} // namespace master
} // namespace uni

#endif // UNI_MASTER_TESTINGCONTEXT_H
