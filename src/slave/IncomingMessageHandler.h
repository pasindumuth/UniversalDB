#ifndef UNI_SLAVE_INCOMINGMESSAGEHANDLER_H
#define UNI_SLAVE_INCOMINGMESSAGEHANDLER_H

#include <net/IncomingMessage.h>
#include <paxos/MultiPaxosHandler.h>
#include <slave/ClientRequestHandler.h>
#include <slave/FailureDetector.h>
#include <slave/LogSyncer.h>

namespace uni {
namespace slave {

class IncomingMessageHandler {
 public:
  IncomingMessageHandler(
      uni::slave::ClientRequestHandler& request_handler,
      uni::slave::FailureDetector& failure_detector,
      uni::slave::LogSyncer& log_syncer,
      uni::paxos::MultiPaxosHandler& multi_paxos_handler);

  void handle(uni::net::IncomingMessage incoming_message);

 private:
  uni::slave::ClientRequestHandler& _client_request_handler;
  uni::slave::FailureDetector& _failure_detector;
  uni::slave::LogSyncer& _log_syncer;
  uni::paxos::MultiPaxosHandler& _multi_paxos_handler;
};

} // namespace slave
} // namespace uni


#endif // UNI_SLAVE_INCOMINGMESSAGEHANDLER_H
