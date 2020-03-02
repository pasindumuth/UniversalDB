#ifndef UNI_SLAVE_TESTINGCONTEXT_H
#define UNI_SLAVE_TESTINGCONTEXT_H

#include <functional>
#include <memory>
#include <thread>
#include <vector>

#include <async/testing/AsyncSchedulerTesting.h>
#include <async/testing/ClockTesting.h>
#include <async/testing/TimerAsyncSchedulerTesting.h>
#include <constants/constants.h>
#include <common/common.h>
#include <net/ConnectionsOut.h>
#include <net/ConnectionsIn.h>
#include <paxos/PaxosLog.h>
#include <slave/FailureDetector.h>
#include <slave/HeartbeatTracker.h>
#include <slave/LogSyncer.h>
#include <slave/ProposerQueue.h>
#include <slave/TabletParticipant.h>
#include <slave/SlaveIncomingMessageHandler.h>

namespace uni {
namespace slave {

struct TestingContext {
  TestingContext(
    uni::constants::Constants const& constants,
    std::string ip_string);

  std::string ip_string;
  uni::async::AsyncSchedulerTesting scheduler;
  uni::net::ConnectionsIn client_connections_in;
  uni::net::ConnectionsIn connections_in;
  uni::net::ConnectionsOut connections_out;
  uni::async::ClockTesting clock;
  uni::async::TimerAsyncSchedulerTesting timer_scheduler;
  uni::slave::HeartbeatTracker heartbeat_tracker;
  uni::slave::FailureDetector failure_detector;
  uni::paxos::PaxosLog paxos_log;
  uni::slave::ProposerQueue proposer_queue;
  uni::paxos::MultiPaxosHandler multipaxos_handler;
  uni::slave::LogSyncer log_syncer;
  uni::slave::SlaveIncomingMessageHandler slave_handler;
};

} // namespace slave
} // namespace uni

#endif // UNI_SLAVE_TESTINGCONTEXT_H
