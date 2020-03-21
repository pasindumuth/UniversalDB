#ifndef UNI_SLAVE_INCOMINGMESSAGEHANDLER_H
#define UNI_SLAVE_INCOMINGMESSAGEHANDLER_H

#include <common/common.h>
#include <net/IncomingMessage.h>
#include <paxos/MultiPaxosHandler.h>
#include <slave/ClientRequestHandler.h>
#include <server/HeartbeatTracker.h>
#include <server/LogSyncer.h>

namespace uni {
namespace slave {

class IncomingMessageHandler {
 public:
  IncomingMessageHandler(
      uni::slave::ClientRequestHandler& request_handler,
      uni::server::LogSyncer& log_syncer,
      uni::paxos::MultiPaxosHandler& multi_paxos_handler);

  void handle(uni::net::IncomingMessage incoming_message);

 private:
  uni::slave::ClientRequestHandler& _client_request_handler;
  uni::server::LogSyncer& _log_syncer;
  uni::paxos::MultiPaxosHandler& _multi_paxos_handler;
};

} // namespace slave
} // namespace uni


#endif // UNI_SLAVE_INCOMINGMESSAGEHANDLER_H
