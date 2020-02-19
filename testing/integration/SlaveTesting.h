#ifndef UNI_TESTING_SLAVETESTING_H
#define UNI_TESTING_SLAVETESTING_H

#include <memory>

#include <async/testing/AsyncSchedulerTesting.h>
#include <async/testing/TimerAsyncSchedulerTesting.h>
#include <common/common.h>
#include <net/ConnectionsOut.h>
#include <paxos/MultiPaxosHandler.h>
#include <paxos/PaxosLog.h>
#include <slave/ClientRequestHandler.h>
#include <slave/HeartbeatTracker.h>
#include <slave/FailureDetector.h>
#include <slave/IncomingMessageHandler.h>
#include <slave/KVStore.h>
#include <slave/LogSyncer.h>
#include <slave/ProposerQueue.h>
#include <slave/TabletId.h>

namespace uni {
namespace testing {
namespace integration {

struct SlaveTesting {
  uni::slave::TabletId tablet_id;
  uni::async::AsyncSchedulerTesting scheduler;
  uni::async::ClockTesting clock;
  uni::async::TimerAsyncSchedulerTesting timer_scheduler;
  uni::net::ConnectionsOut connections_out;
  uni::paxos::PaxosLog paxos_log;
  uni::paxos::MultiPaxosHandler multipaxos_handler;
  uni::slave::ProposerQueue proposer_queue;
  uni::slave::KVStore kvstore;
  uni::slave::ClientRequestHandler client_request_handler;
  uni::slave::HeartbeatTracker heartbeat_tracker;
  uni::slave::FailureDetector failure_detector;
  uni::slave::LogSyncer log_syncer;
  uni::slave::IncomingMessageHandler incoming_message_handler;

  SlaveTesting(
    uni::slave::TabletId& tid,
    uni::constants::Constants const& constants);
};

} // integration
} // testing
} // uni


#endif // UNI_TESTING_SLAVETESTING_H
