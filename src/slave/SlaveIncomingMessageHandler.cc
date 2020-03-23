#include "SlaveIncomingMessageHandler.h"

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/optional.hpp>

#include <common/common.h>
#include <net/EndpointId.h>
#include <net/IncomingMessage.h>
#include <proto/client.pb.h>
#include <proto/master.pb.h>
#include <proto/message.pb.h>
#include <proto/tablet.pb.h>
#include <server/LogSyncer.h>
#include <slave/TabletId.h>
#include <slave/TabletParticipant.h>

namespace uni {
namespace slave {

SlaveIncomingMessageHandler::SlaveIncomingMessageHandler(
  std::function<uni::custom_unique_ptr<uni::slave::TabletParticipant>(uni::slave::TabletId)> tp_provider,
  uni::server::HeartbeatTracker& heartbeat_tracker,
  uni::server::LogSyncer& log_syncer)
  : _tp_provider(tp_provider),
    _heartbeat_tracker(heartbeat_tracker),
    _log_syncer(log_syncer) {}

void SlaveIncomingMessageHandler::handle(uni::net::IncomingMessage incoming_message) {
  auto endpoint_id = incoming_message.endpoint_id;
  auto message_wrapper = proto::message::MessageWrapper();
  message_wrapper.ParseFromString(incoming_message.message);
  if (message_wrapper.has_client_message()) {
    auto const& client_message = message_wrapper.client_message();
    if (client_message.has_request()) {
      LOG(uni::logging::Level::TRACE2, "Client Request at SlaveIncomingMessageHandler gotten.")
      forward_message(
        client_message.request().database_id().value(),
        client_message.request().table_id().value(),
        incoming_message);
    }
  } else if (message_wrapper.has_tablet_message()) {
    LOG(uni::logging::Level::TRACE2, "TabletMessage at SlaveIncomingMessageHandler gotten.")
    auto tablet_message = message_wrapper.tablet_message();
    forward_message(
      tablet_message.database_id().value(),
      tablet_message.table_id().value(),
      incoming_message);
  } else if (message_wrapper.has_slave_message()) {
    auto const& slave_message = message_wrapper.slave_message();
    if (slave_message.has_heartbeat()) {
      LOG(uni::logging::Level::TRACE2, "Heartbeat at SlaveIncomingMessageHandler gotten.")
      _heartbeat_tracker.handle_heartbeat(endpoint_id);
    } else if (slave_message.has_sync_message()) {
      auto sync_message = slave_message.sync_message();
      if (sync_message.has_sync_request()) {
        LOG(uni::logging::Level::TRACE2, "Sync Request at SlaveIncomingMessageHandler gotten.")
        _log_syncer.handle_sync_request(endpoint_id, sync_message.sync_request());
      } else if (sync_message.has_sync_response()) {
        LOG(uni::logging::Level::TRACE2, "Sync Response at SlaveIncomingMessageHandler gotten.")
        _log_syncer.handle_sync_response(sync_message.sync_response());
      }
    }
  }
}

std::unordered_map<TabletId, uni::custom_unique_ptr<uni::slave::TabletParticipant>> const& SlaveIncomingMessageHandler::get_tps() const {
  return _tp_map;
}

void SlaveIncomingMessageHandler::forward_message(
  std::string database_id,
  std::string table_id,
  uni::net::IncomingMessage incoming_message
) {
  TabletId tablet_id = { database_id, table_id, boost::none, boost::none};
  if (_tp_map.find(tablet_id) == _tp_map.end()) {
    _tp_map.insert({tablet_id, _tp_provider(tablet_id)});
  }
  _tp_map[tablet_id]->scheduler->queue_message(incoming_message);
}

} // namespace slave
} // namespace uni
