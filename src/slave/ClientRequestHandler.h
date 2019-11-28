#ifndef UNI_SLAVE_CLIENTREQUESTHANDLER
#define UNI_SLAVE_CLIENTREQUESTHANDLER

#include <net/endpoint_id.h>
#include <paxos/MultiPaxosHandler.h>
#include <proto/client.pb.h>
#include <slave/ProposerQueue.h>

namespace uni {
namespace slave {

class ClientRequestHandler {
 public:
  ClientRequestHandler(
      uni::paxos::MultiPaxosHandler& multi_paxos_handler,
      uni::slave::ProposerQueue& proposer_queue);

  void handle_request(uni::net::endpoint_id const& endpoint_id, proto::client::ClientRequest const& message);

 private:
  uni::paxos::MultiPaxosHandler& _multi_paxos_handler;
  uni::slave::ProposerQueue& _proposer_queue;
};

} // namespace slave
} // namespace uni

#endif // UNI_SLAVE_CLIENTREQUESTHANDLER
