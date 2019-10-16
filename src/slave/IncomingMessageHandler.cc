#include "IncomingMessageHandler.h"

#include <proto/message.pb.h>

namespace uni {
namespace slave {

using proto::message::MessageWrapper;
using uni::net::IncomingMessage;
using uni::paxos::MultiPaxosHandler;
using uni::slave::ClientRequestHandler;
using uni::slave::FailureDetector;

IncomingMessageHandler::IncomingMessageHandler(
    ClientRequestHandler& request_handler,
    FailureDetector& failure_detector,
    MultiPaxosHandler& multi_paxos_handler)
      : _client_request_handler(request_handler),
        _failure_detector(failure_detector),
        _multi_paxos_handler(multi_paxos_handler) {}

void IncomingMessageHandler::handle(IncomingMessage incoming_message) {
  auto message_wrapper = MessageWrapper();
  message_wrapper.ParseFromString(incoming_message.message);
  if (message_wrapper.has_client_message()) {
    auto const& client_message = message_wrapper.client_message();
    if (client_message.has_request()) {
      _client_request_handler.handle_request(incoming_message.endpoint_id, client_message.request());
    }
  } else if (message_wrapper.has_paxos_message()) {
    _multi_paxos_handler.handle_incoming_message(incoming_message.endpoint_id, message_wrapper.paxos_message());
  } else if (message_wrapper.has_slave_message()) {
    _failure_detector.handle_heartbeat(incoming_message.endpoint_id);
  }
}

} // namespace slave
} // namespace uni
