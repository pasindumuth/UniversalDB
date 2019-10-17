#ifndef UNI_TESTING_SLAVETESTING_H
#define UNI_TESTING_SLAVETESTING_H

#include <memory>

#include <async/testing/AsyncSchedulerTesting.h>
#include <async/testing/TimerAsyncSchedulerTesting.h>
#include <net/ConnectionsOut.h>
#include <paxos/MultiPaxosHandler.h>
#include <paxos/PaxosLog.h>
#include <slave/ClientRequestHandler.h>
#include <slave/FailureDetector.h>
#include <slave/IncomingMessageHandler.h>

namespace uni {
namespace testing {

struct SlaveTesting {
  std::unique_ptr<uni::async::AsyncSchedulerTesting> scheduler;
  std::unique_ptr<uni::async::ClockTesting> clock;
  std::unique_ptr<uni::async::TimerAsyncSchedulerTesting> timer_scheduler;
  std::unique_ptr<uni::net::ConnectionsOut> connections_out;
  std::unique_ptr<uni::paxos::MultiPaxosHandler> multipaxos_handler;
  std::unique_ptr<uni::paxos::PaxosLog> paxos_log;
  std::unique_ptr<uni::slave::ClientRequestHandler> client_request_handler;
  std::unique_ptr<uni::slave::FailureDetector> failure_detector;
  std::unique_ptr<uni::slave::IncomingMessageHandler> incoming_message_handler;
};

} // testing
} // uni


#endif // UNI_TESTING_SLAVETESTING_H
