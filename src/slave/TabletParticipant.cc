#include "TabletParticipant.h"

#include <net/endpoint_id.h>
#include <net/IncomingMessage.h>
#include <paxos/PaxosTypes.h>
#include <paxos/SinglePaxosHandler.h>
#include <proto/client.pb.h>
#include <proto/master.pb.h>
#include <proto/message.pb.h>
#include <proto/tablet.pb.h>
#include <proto/sync.pb.h>
#include <utils/pbutil.h>

namespace uni {
namespace slave {

TabletParticipant::TabletParticipant(
  std::function<std::unique_ptr<uni::async::AsyncScheduler>()> scheduler_provider,
  uni::constants::Constants const& constants,
  uni::net::Connections& connections,
  uni::net::Connections& client_connections,
  uni::async::TimerAsyncScheduler& timer_scheduler,
  uni::server::FailureDetector& failure_detector,
  uni::slave::TabletId& tid)
  : scheduler(scheduler_provider()),
    tablet_id(tid),
    paxos_log(),
    multipaxos_handler(
      paxos_log,
      [this, &constants, &connections](uni::paxos::index_t index) {
        return uni::paxos::SinglePaxosHandler(
          constants,
          connections,
          paxos_log,
          index,
          [this](proto::paxos::PaxosMessage* paxos_message){
            auto message_wrapper = proto::message::MessageWrapper();
            auto tablet_message = new proto::tablet::TabletMessage;
            tablet_message->set_allocated_database_id(uni::utils::pb::string(tablet_id.database_id));
            tablet_message->set_allocated_table_id(uni::utils::pb::string(tablet_id.table_id));
            tablet_message->set_allocated_paxos_message(paxos_message);
            message_wrapper.set_allocated_tablet_message(tablet_message);
            return message_wrapper;
          });
      }),
    proposer_queue(timer_scheduler),
    kvstore(),
    client_request_handler(
      multipaxos_handler,
      paxos_log,
      proposer_queue,
      kvstore,
      [this, &client_connections](
        uni::net::endpoint_id endpoint_id,
        proto::client::ClientResponse* client_response
      ) {
        auto message_wrapper = proto::message::MessageWrapper();
        auto client_message = new proto::client::ClientMessage();
        client_message->set_allocated_response(client_response);
        message_wrapper.set_allocated_client_message(client_message);
        auto channel = client_connections.get_channel(endpoint_id);
        if (channel) {
          channel.get().queue_send(message_wrapper.SerializeAsString());
        } else {
          LOG(uni::logging::Level::WARN, "Client Channel to reply to no longer exists.");
        }
      }),
    log_syncer(
      constants,
      connections,
      timer_scheduler,
      paxos_log,
      [this](proto::sync::SyncMessage* sync_message){
        auto message_wrapper = proto::message::MessageWrapper();
        auto tablet_message = new proto::tablet::TabletMessage;
        tablet_message->set_allocated_database_id(uni::utils::pb::string(tablet_id.database_id));
        tablet_message->set_allocated_table_id(uni::utils::pb::string(tablet_id.table_id));
        tablet_message->set_allocated_sync_message(sync_message);
        message_wrapper.set_allocated_tablet_message(tablet_message);
        return message_wrapper;
      }),
    incoming_message_handler(
      client_request_handler,
      log_syncer,
      multipaxos_handler) {
  scheduler->set_callback([this](uni::net::IncomingMessage message){
    incoming_message_handler.handle(message);
  });
  paxos_log.add_callback(kvstore.get_paxos_callback());
}

} // namespace slave
} // namespace uni
