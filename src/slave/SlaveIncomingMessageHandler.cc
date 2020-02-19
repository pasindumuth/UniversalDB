#include "SlaveIncomingMessageHandler.h"

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

#include <async/impl/AsyncSchedulerImpl.h>
#include <async/impl/TimerAsyncSchedulerImpl.h>
#include <common/common.h>
#include <constants/constants.h>
#include <net/ConnectionsIn.h>
#include <net/endpoint_id.h>
#include <net/IncomingMessage.h>
#include <net/impl/ChannelImpl.h>
#include <proto/client.pb.h>
#include <proto/master.pb.h>
#include <proto/message.pb.h>
#include <proto/tablet.pb.h>
#include <paxos/MultiPaxosHandler.h>
#include <paxos/PaxosLog.h>
#include <paxos/PaxosTypes.h>
#include <paxos/SinglePaxosHandler.h>
#include <slave/ClientConnectionHandler.h>
#include <slave/ClientRequestHandler.h>
#include <slave/IncomingMessageHandler.h>
#include <slave/KVStore.h>
#include <slave/LogSyncer.h>
#include <slave/ProposerQueue.h>
#include <slave/ServerConnectionHandler.h>

namespace uni {
namespace slave {

SlaveIncomingMessageHandler::SlaveIncomingMessageHandler(
  uni::net::ConnectionsIn& client_connections_in,
  uni::net::ConnectionsOut& connections_out,
  uni::slave::FailureDetector& failure_detector,
  uni::slave::HeartbeatTracker& heartbeat_tracker,
  uni::slave::LogSyncer& log_syncer,
  uni::constants::Constants const& constants,
  uni::async::TimerAsyncScheduler& timer_scheduler)
  : _client_connections_in(client_connections_in),
    _connections_out(connections_out),
    _failure_detector(failure_detector),
    _heartbeat_tracker(heartbeat_tracker),
    _log_syncer(log_syncer),
    _constants(constants),
    _timer_scheduler(timer_scheduler) {}

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

// TODO move all this to another class so that the dependencies (fd, ci, cci, co) can all
// be isolated from the request routing logic this class is meant to only handle.
// Also, don't construct the TP on the stack, since thread stacks and TPs need to be 
// decoupled going forward. Instantiate everything below by valud in a struct, where
// all the code below is in the constructor.
void SlaveIncomingMessageHandler::addTablet(TabletId tablet_id) {
  _tp_map.insert({
    tablet_id,
    std::make_unique<uni::slave::TabletParticipant>(
      _constants,
      _connections_out,
      _client_connections_in,
      _timer_scheduler,
      _failure_detector
    )
  });
}

void SlaveIncomingMessageHandler::forward_message(
  std::string database_id,
  std::string table_id,
  uni::net::IncomingMessage incoming_message
) {
  TabletId tabletId = { database_id, table_id, "", ""};
  if (_tp_map.find(tabletId) == _tp_map.end()) {
    // We have to add a Tablet with the given tabletId
    addTablet(tabletId);
  }

  auto& tp = _tp_map[tabletId];
  tp->scheduler.queue_message(incoming_message);
}

} // namespace slave
} // namespace uni
