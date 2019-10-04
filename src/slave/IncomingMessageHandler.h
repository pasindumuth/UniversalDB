#ifndef UNI_SLAVE_INCOMINGMESSAGEHANDLER_H
#define UNI_SLAVE_INCOMINGMESSAGEHANDLER_H

#include <memory>

#include <net/IncomingMessage.h>
#include <paxos/MultiPaxosHandler.h>
#include <slave/ClientRequestHandler.h>

namespace uni {
namespace slave {

class IncomingMessageHandler {
 public:
  IncomingMessageHandler(
      std::shared_ptr<uni::slave::ClientRequestHandler> request_handler,
      std::shared_ptr<uni::paxos::MultiPaxosHandler> multi_paxos_handler);

  void handle(uni::net::IncomingMessage incoming_message);

 private:
  std::shared_ptr<uni::slave::ClientRequestHandler> _client_request_handler;
  std::shared_ptr<uni::paxos::MultiPaxosHandler> _multi_paxos_handler;
};

} // namespace slave
} // namespace uni


#endif // UNI_SLAVE_INCOMINGMESSAGEHANDLER_H
