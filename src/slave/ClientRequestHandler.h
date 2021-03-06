#ifndef UNI_SLAVE_CLIENTREQUESTHANDLER
#define UNI_SLAVE_CLIENTREQUESTHANDLER

#include <functional>
#include <unordered_map>

#include <common/common.h>
#include <net/EndpointId.h>
#include <paxos/MultiPaxosHandler.h>
#include <paxos/PaxosLog.h>
#include <paxos/PaxosTypes.h>
#include <async/AsyncQueue.h>
#include <slave/KVStore.h>
#include <slave/functors.h>

#include <proto/message_client.pb.h>

namespace uni {
namespace slave {

class ClientRequestHandler {
 public:
  static int const RETRY_LIMIT;
  static int const WAIT_FOR_PAXOS;

  ClientRequestHandler(
    uni::paxos::MultiPaxosHandler& multi_paxos_handler,
    uni::paxos::PaxosLog& paxos_log,
    uni::async::AsyncQueue& async_queue,
    uni::slave::KVStore& kvstore,
    uni::slave::ClientRespond respond_callback);

  void handle_request(
    uni::net::EndpointId endpoint_id,
    proto::message::client::ClientRequest const& message);

 private:
  uni::paxos::MultiPaxosHandler& _multi_paxos_handler;
  uni::paxos::PaxosLog& _paxos_log;
  uni::async::AsyncQueue& _async_queue;
  uni::slave::KVStore& _kvstore;
  uni::slave::ClientRespond _respond_callback;

  // Another thing to maintain, another thing that could result in a memory leak.
  std::unordered_map<std::string, uni::paxos::index_t> _request_id_map;
};

} // namespace slave
} // namespace uni

#endif // UNI_SLAVE_CLIENTREQUESTHANDLER
