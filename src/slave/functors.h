#ifndef UNI_SLAVE_FUNCTORS
#define UNI_SLAVE_FUNCTORS

#include <vector>

#include <common/common.h>
#include <net/Connections.h>
#include <net/EndpointId.h>
#include <proto/client.pb.h>
#include <proto/message.pb.h>
#include <proto/paxos.pb.h>
#include <slave/SlaveConfigManager.h>

namespace uni {
namespace slave {

struct SendPaxos {
  proto::message::MessageWrapper operator()(proto::paxos::PaxosMessage* paxos_message){
    auto message_wrapper = proto::message::MessageWrapper();
    auto slave_message = new proto::slave::SlaveMessage;
    slave_message->set_allocated_paxos_message(paxos_message);
    message_wrapper.set_allocated_slave_message(slave_message);
    return message_wrapper;
  }
};

struct SendSync {
  proto::message::MessageWrapper operator()(proto::sync::SyncMessage* sync_message){
    auto message_wrapper = proto::message::MessageWrapper();
    auto slave_message = new proto::slave::SlaveMessage;
    slave_message->set_allocated_sync_message(sync_message);
    message_wrapper.set_allocated_slave_message(slave_message);
    return message_wrapper;
  }
};

struct GetEndpoints {
  GetEndpoints(uni::slave::SlaveConfigManager& config_manager):
    _config_manager(config_manager) {}

  std::vector<uni::net::EndpointId> operator()(){
    return _config_manager.config_endpoints();
  }

 private:
  uni::slave::SlaveConfigManager& _config_manager;
};

} // namespace slave
} // namespace uni

#endif // UNI_SLAVE_FUNCTORS
