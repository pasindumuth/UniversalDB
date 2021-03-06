#include "TabletParticipantManager.h"

#include <net/EndpointId.h>
#include <proto/message_client.pb.h>
#include <proto/message_master.pb.h>
#include <proto/message.pb.h>
#include <proto/message_tablet.pb.h>

namespace uni {
namespace slave {

TabletParticipantManager::TabletParticipantManager(
  std::function<uni::custom_unique_ptr<uni::slave::TabletParticipant>(uni::slave::TabletId)> tp_provider,
  uni::net::Connections& client_connections,
  uni::slave::ClientRespond respond_callback)
  : _tp_provider(tp_provider),
    _client_connections(client_connections),
    _respond_callback(respond_callback) {}

void TabletParticipantManager::handle(uni::net::IncomingMessage incoming_message) {
  auto endpoint_id = incoming_message.endpoint_id;
  auto message_wrapper = proto::message::MessageWrapper();
  message_wrapper.ParseFromString(incoming_message.message);
  if (message_wrapper.has_client_message()) {
    auto const& client_message = message_wrapper.client_message();
    if (client_message.has_request()) {
      LOG(uni::logging::Level::TRACE2, "Client Request at TabletParticipantManager gotten.")
      auto client_request = client_message.request();
      handle_client_request(client_request, incoming_message);
    } else {
      LOG(uni::logging::Level::WARN, "Unkown client message type.")
    }
  } else if (message_wrapper.has_tablet_message()) {
    LOG(uni::logging::Level::TRACE2, "TabletMessage at TabletParticipantManager gotten.")
    auto tablet_message = message_wrapper.tablet_message();
    handle_tablet_message(tablet_message, incoming_message);
  } else {
    LOG(uni::logging::Level::WARN, "Unkown message type in TabletParticipantManager.")
  }
}

void TabletParticipantManager::handle_key_space_change(std::vector<uni::server::KeySpaceRange> const& ranges) {
  for (auto const& range: ranges) {
    auto it = _tp_map.find(range);
    if (it == _tp_map.end()) {
      _tp_map.insert({ range, _tp_provider(range) });
    }
  }
}

TabletParticipantManager::TPMapT const& TabletParticipantManager::get_tps() const {
  return _tp_map;
}

void TabletParticipantManager::handle_client_request(
  proto::message::client::ClientRequest client_request,
  uni::net::IncomingMessage incoming_message)
{
  for (auto& [tablet_id, tp] : _tp_map) {
    if (uni::server::within_range(tablet_id, client_request)) {
      tp->_scheduler->queue_message(incoming_message);
      return;
    }
  }
  // If we get here, that means there is no TabletParticipant that supports
  // this client message. So send back an error.
  auto client_response = new proto::message::client::ClientResponse();
  client_response->set_error_code(proto::message::client::Code::ERROR);
  _respond_callback(incoming_message.endpoint_id, client_response);
}

void TabletParticipantManager::handle_tablet_message(
  proto::message::tablet::TabletMessage tablet_message,
  uni::net::IncomingMessage incoming_message)
{
  for (auto& [tablet_id, tp] : _tp_map) {
    if (tablet_id == uni::server::convert(tablet_message.range())) {
      tp->_scheduler->queue_message(incoming_message);
      return;
    }
  }
}

} // namespace slave
} // namespace uni
