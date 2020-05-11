#include "TabletParticipant.h"

#include <net/EndpointId.h>
#include <net/IncomingMessage.h>
#include <paxos/PaxosTypes.h>
#include <paxos/SinglePaxosHandler.h>
#include <proto/message_client.pb.h>
#include <proto/message_master.pb.h>
#include <proto/message.pb.h>
#include <proto/message_tablet.pb.h>
#include <proto/sync.pb.h>
#include <slave/functors.h>
#include <utils/pbutil.h>

namespace uni {
namespace slave {

TabletParticipant::TabletParticipant(
  std::unique_ptr<uni::async::AsyncScheduler> scheduler,
  std::unique_ptr<uni::async::TimerAsyncScheduler> timer_scheduler,
  std::unique_ptr<uni::random::Random> random,
  uni::constants::Constants const& constants,
  uni::net::Connections& slave_connections,
  uni::net::Connections& client_connections,
  uni::server::FailureDetector& failure_detector,
  uni::slave::SlaveConfigManager& config_manager,
  uni::slave::TabletId const& tid)
  : _scheduler(std::move(scheduler)),
    _timer_scheduler(std::move(timer_scheduler)),
    _random(std::move(random)),
    _tablet_id(tid),
    _paxos_log(),
    _paxos_config_manager(
      0,
      slave_connections.get_all_endpoints(),
      _paxos_log),
    _multipaxos_handler(
      _paxos_log,
      [this, &constants, &slave_connections, &config_manager](uni::paxos::index_t index) {
        return uni::paxos::SinglePaxosHandler(
          constants,
          slave_connections,
          _paxos_log,
          *_random,
          index,
          _paxos_config_manager.config(index),
          [this](proto::paxos::PaxosMessage* paxos_message){
            auto message_wrapper = proto::message::MessageWrapper();
            auto tablet_message = new proto::message::tablet::TabletMessage;
            tablet_message->set_allocated_range(uni::server::convert(_tablet_id));
            tablet_message->set_allocated_paxos_message(paxos_message);
            message_wrapper.set_allocated_tablet_message(tablet_message);
            return message_wrapper;
          });
      }),
    _async_queue(*_timer_scheduler),
    _kvstore(),
    _client_request_handler(
      _multipaxos_handler,
      _paxos_log,
      _async_queue,
      _kvstore,
      uni::slave::ClientRespond(client_connections)),
    _log_syncer(
      constants,
      slave_connections,
      *_timer_scheduler,
      _paxos_log,
      [this](){ return _paxos_config_manager.latest_config(); },
      [this](proto::sync::SyncMessage* sync_message){
        auto message_wrapper = proto::message::MessageWrapper();
        auto tablet_message = new proto::message::tablet::TabletMessage;
        tablet_message->set_allocated_range(uni::server::convert(_tablet_id));
        tablet_message->set_allocated_sync_message(sync_message);
        message_wrapper.set_allocated_tablet_message(tablet_message);
        return message_wrapper;
      }),
    _incoming_message_handler(
      _client_request_handler,
      _log_syncer,
      _multipaxos_handler)
{
  _scheduler->set_callback([this](uni::net::IncomingMessage message){
    _incoming_message_handler.handle(message);
  });
}

} // namespace slave
} // namespace uni
