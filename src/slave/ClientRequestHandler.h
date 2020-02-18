#ifndef UNI_SLAVE_CLIENTREQUESTHANDLER
#define UNI_SLAVE_CLIENTREQUESTHANDLER

#include <functional>
#include <unordered_map>

#include <common/common.h>
#include <net/endpoint_id.h>
#include <paxos/MultiPaxosHandler.h>
#include <paxos/PaxosLog.h>
#include <paxos/PaxosTypes.h>
#include <slave/ProposerQueue.h>
#include <slave/KVStore.h>

#include <proto/client.pb.h>

namespace uni {
namespace slave {

class ClientRequestHandler {
 public:
  ClientRequestHandler(
      uni::paxos::MultiPaxosHandler& multi_paxos_handler,
      uni::paxos::PaxosLog& paxos_log,
      uni::slave::ProposerQueue& proposer_queue,
      uni::slave::KVStore& kvstore,
      std::function<void(uni::net::endpoint_id, proto::client::ClientResponse*)> respond);

  void handle_request(
    uni::net::endpoint_id endpoint_id,
    proto::client::ClientRequest const& message);

 private:
  uni::paxos::MultiPaxosHandler& _multi_paxos_handler;
  uni::paxos::PaxosLog& _paxos_log;
  uni::slave::ProposerQueue& _proposer_queue;
  uni::slave::KVStore& _kvstore;
  // This function consumes the ClientResponse; it deletes it from memory
  std::function<void(uni::net::endpoint_id, proto::client::ClientResponse*)> _respond;

  // Another thing to maintain, another thing that could result in a memory leak.
  std::unordered_map<std::string, uni::paxos::index_t> _request_id_map;
};

} // namespace slave
} // namespace uni

#endif // UNI_SLAVE_CLIENTREQUESTHANDLER
