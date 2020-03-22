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
    std::vector<uni::net::EndpointId>& config_endpoints,
    std::string ip_string);

  std::string ip_string;
  uni::async::AsyncSchedulerTesting scheduler;
  uni::net::Connections client_connections;
  uni::net::Connections master_connections;
  uni::net::Connections connections;
  uni::async::ClockTesting clock;
  uni::async::TimerAsyncSchedulerTesting timer_scheduler;
  uni::server::HeartbeatTracker heartbeat_tracker;
  uni::server::FailureDetector failure_detector;
  uni::paxos::PaxosLog paxos_log;
  uni::async::AsyncQueue async_queue;
  uni::paxos::MultiPaxosHandler multipaxos_handler;
  uni::server::LogSyncer log_syncer;
  uni::slave::SlaveConfigManager config_manager;
  uni::slave::SlaveKeySpaceManager key_space_manager;
  uni::slave::SlaveIncomingMessageHandler slave_handler;
};

} // namespace slave
} // namespace uni

#endif // UNI_SLAVE_TESTINGCONTEXT_H
