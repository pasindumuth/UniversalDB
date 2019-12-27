#ifndef UNI_SLAVE_CLIENTREQUESTHANDLER
#define UNI_SLAVE_CLIENTREQUESTHANDLER

#include <functional>
#include <unordered_map>

#include <common/common.h>
#include <net/endpoint_id.h>
#include <paxos/MultiPaxosHandler.h>
#include <paxos/PaxosLog.h>
#include <paxos/PaxosTypes.h>
#include <proto/client.pb.h>
#include <slave/ProposerQueue.h>

namespace uni {
namespace slave {

class ClientRequestHandler {
 public:
  ClientRequestHandler(
      uni::paxos::MultiPaxosHandler& multi_paxos_handler,
      uni::paxos::PaxosLog& paxos_log,
      uni::slave::ProposerQueue& proposer_queue);

  void handle_request(
    std::function<void(proto::client::ClientResponse*)> response_callback,
    proto::client::ClientRequest const& message);

 private:
  uni::paxos::MultiPaxosHandler& _multi_paxos_handler;
  uni::paxos::PaxosLog& _paxos_log;
  uni::slave::ProposerQueue& _proposer_queue;

  // Another thing to maintain, another thing that could result in a memory leak.
  std::unordered_map<std::string, uni::paxos::index_t> _request_id_map;
};

} // namespace slave
} // namespace uni

#endif // UNI_SLAVE_CLIENTREQUESTHANDLER
