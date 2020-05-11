#ifndef UNI_SLAVE_FUNCTORS
#define UNI_SLAVE_FUNCTORS

#include <vector>

#include <common/common.h>
#include <net/Connections.h>
#include <net/EndpointId.h>
#include <proto/message_client.pb.h>
#include <proto/message.pb.h>
#include <proto/paxos.pb.h>
#include <slave/SlaveConfigManager.h>

namespace uni {
namespace slave {

struct SendPaxos {
  proto::message::MessageWrapper operator()(proto::paxos::PaxosMessage* paxos_message){
    auto message_wrapper = proto::message::MessageWrapper();
    auto slave_message = new proto::message::slave::SlaveMessage;
    slave_message->set_allocated_paxos_message(paxos_message);
    message_wrapper.set_allocated_slave_message(slave_message);
    return message_wrapper;
  }
};

struct SendSync {
  proto::message::MessageWrapper operator()(proto::sync::SyncMessage* sync_message){
    auto message_wrapper = proto::message::MessageWrapper();
    auto slave_message = new proto::message::slave::SlaveMessage;
    slave_message->set_allocated_sync_message(sync_message);
    message_wrapper.set_allocated_slave_message(slave_message);
    return message_wrapper;
  }
};

// This function consumes the ClientResponse; it deletes it from memory
struct ClientRespond {
  ClientRespond(uni::net::Connections& client_connections):
    _client_connections(client_connections) {}

  void operator()(
    uni::net::EndpointId endpoint_id,
    proto::message::client::ClientResponse* client_response
  ) {
    auto message_wrapper = proto::message::MessageWrapper();
    auto client_message = new proto::message::client::ClientMessage();
    client_message->set_allocated_response(client_response);
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

} // namespace slave
} // namespace uni

#endif // UNI_SLAVE_FUNCTORS
