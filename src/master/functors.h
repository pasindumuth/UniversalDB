#ifndef UNI_MASTER_FUNCTORS
#define UNI_MASTER_FUNCTORS

#include <common/common.h>
#include <net/Connections.h>
#include <net/EndpointId.h>
#include <proto/client.pb.h>
#include <proto/message.pb.h>
#include <proto/paxos.pb.h>

namespace uni {
namespace master {

struct SendFindKeyRangeResponse {
  SendFindKeyRangeResponse(uni::net::Connections& client_connections):
    _client_connections(client_connections) {}

  void operator()(
    uni::net::EndpointId endpoint_id,
    proto::client::FindKeyRangeResponse* find_key_space_response
  ) {
    auto message_wrapper = proto::message::MessageWrapper();
    auto client_message = new proto::client::ClientMessage();
    client_message->set_allocated_find_key_range_response(find_key_space_response);
    message_wrapper.set_allocated_client_message(client_message);
    auto channel = _client_connections.get_channel(endpoint_id);
    if (channel) {
      channel.get().queue_send(message_wrapper.SerializeAsString());
    } else {
      LOG(uni::logging::Level::WARN, "Client Channel to reply to no longer exists.");
    }
  }

 private:
  uni::net::Connections& _client_connections;
};

} // namespace master
} // namespace uni

#endif // UNI_MASTER_FUNCTORS
