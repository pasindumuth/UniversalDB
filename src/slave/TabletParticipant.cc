#include "TabletParticipant.h"

#include <net/EndpointId.h>
#include <net/IncomingMessage.h>
#include <paxos/PaxosTypes.h>
#include <paxos/SinglePaxosHandler.h>
#include <proto/client.pb.h>
#include <proto/master.pb.h>
#include <proto/message.pb.h>
#include <proto/tablet.pb.h>
#include <proto/sync.pb.h>
#include <slave/functors.h>
#include <utils/pbutil.h>

namespace uni {
namespace slave {

TabletParticipant::TabletParticipant(
  std::function<std::unique_ptr<uni::async::AsyncScheduler>()> scheduler_provider,
  std::unique_ptr<uni::random::Random> random,
  uni::constants::Constants const& constants,
  uni::net::Connections& connections,
  uni::net::Connections& client_connections,
  uni::async::TimerAsyncScheduler& timer_scheduler,
  uni::server::FailureDetector& failure_detector,
  uni::slave::SlaveConfigManager& config_manager,
  uni::slave::TabletId& tid)
  : _scheduler(scheduler_provider()),
    _random(std::move(random)),
    _tablet_id(tid),
    _paxos_log(),
    _multipaxos_handler(
      _paxos_log,
      [this, &constants, &connections, &config_manager](uni::paxos::index_t index) {
        return uni::paxos::SinglePaxosHandler(
          constants,
          connections,
          _paxos_log,
          *_random,
          index,
          uni::slave::GetEndpoints(config_manager),
          [this](proto::paxos::PaxosMessage* paxos_message){
            auto message_wrapper = proto::message::MessageWrapper();
            auto tablet_message = new proto::tablet::TabletMessage;
            tablet_message->set_allocated_database_id(uni::utils::pb::string(_tablet_id.database_id));
            tablet_message->set_allocated_table_id(uni::utils::pb::string(_tablet_id.table_id));
            tablet_message->set_allocated_paxos_message(paxos_message);
            message_wrapper.set_allocated_tablet_message(tablet_message);
            return message_wrapper;
          });
      }),
    _async_queue(timer_scheduler),
    _kvstore(),
    _client_request_handler(
      _multipaxos_handler,
      _paxos_log,
      _async_queue,
      _kvstore,
      [&client_connections](
        uni::net::EndpointId endpoint_id,
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
    _log_syncer(
      constants,
      connections,
      timer_scheduler,
      _paxos_log,
      uni::slave::GetEndpoints(config_manager),
      [this](proto::sync::SyncMessage* sync_message){
        auto message_wrapper = proto::message::MessageWrapper();
        auto tablet_message = new proto::tablet::TabletMessage;
        tablet_message->set_allocated_database_id(uni::utils::pb::string(_tablet_id.database_id));
        tablet_message->set_allocated_table_id(uni::utils::pb::string(_tablet_id.table_id));
        tablet_message->set_allocated_sync_message(sync_message);
        message_wrapper.set_allocated_tablet_message(tablet_message);
        return message_wrapper;
      }),
    _incoming_message_handler(
      _client_request_handler,
      _log_syncer,
      _multipaxos_handler) {
  _scheduler->set_callback([this](uni::net::IncomingMessage message){
    _incoming_message_handler.handle(message);
  });
  _paxos_log.add_callback(_kvstore.get_paxos_callback());
}

} // namespace slave
} // namespace uni
