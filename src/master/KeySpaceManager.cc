#include "KeySpaceManager.h"

#include <boost/optional.hpp>

#include <common/common.h>
#include <utils/pbutil.h>

namespace uni {
namespace master {

KeySpaceManager::KeySpaceManager(
  uni::async::AsyncQueue& async_queue,
  uni::master::GroupConfigManager& config_manager,
  uni::net::Connections& slave_connections,
  uni::paxos::MultiPaxosHandler& multipaxos_handler,
  uni::paxos::PaxosLog& paxos_log,
  uni::master::SendFindKeyRangeResponse respond):
    _async_queue(async_queue),
    _config_manager(config_manager),
    _slave_connections(slave_connections),
    _multipaxos_handler(multipaxos_handler),
    _paxos_log(paxos_log),
    _respond(respond) {}

void KeySpaceManager::set_first_config(uni::server::SlaveGroupId group_id) {
  _slave_group_ranges.insert({group_id, KeySpace{{}, 0}});
}

proto::message::MessageWrapper KeySpaceManager::build_new_key_space_selected_message(NewKeySpace const& key_space) {
  auto message_wrapper = proto::message::MessageWrapper();
  auto master_message = new proto::master::MasterMessage();
  auto key_space_selected_message = new proto::master::NewKeySpaceSelected();
  for (auto const& range : key_space.new_ranges) {
    auto const proto_range = key_space_selected_message->add_new_ranges();
    proto_range->set_database_id(range.database_id);
    proto_range->set_table_id(range.table_id);
    if (range.start_key) proto_range->set_allocated_start_key(uni::utils::pb::string(range.start_key.get()));
    if (range.end_key) proto_range->set_allocated_end_key(uni::utils::pb::string(range.end_key.get()));
  }
  key_space_selected_message->set_generation(key_space.generation);
  master_message->set_allocated_key_space_selected(key_space_selected_message);
  message_wrapper.set_allocated_master_message(master_message);
  return message_wrapper;
}

bool KeySpaceManager::within_range(
  uni::server::KeySpaceRange const& range,
  proto::client::FindKeyRangeRequest const& message
) {
  if (range.database_id != message.database_id()) return false;
  if (range.table_id != message.table_id()) return false;
  if (range.start_key != boost::none && range.start_key.get() > message.key()) return false;
  if (range.end_key != boost::none && range.end_key.get() <= message.key()) return false;
  return true;
}

void KeySpaceManager::handle_find_key(
  uni::net::EndpointId endpoint_id,
  proto::client::FindKeyRangeRequest const& message
) {
  auto retry_count = std::make_shared<int>(0);
  _async_queue.add_task([this, retry_count, message, endpoint_id]() {
    // Search the key space ranges to see if there is a slave group
    // that has the desired key range. If it does, reply to the client.
    for (auto const& [group_id, v] : _slave_group_ranges) {
      if (auto const& key_space = std::get_if<KeySpace>(&v)) {
        for (auto const& range : key_space->ranges) {
          if (within_range(range, message)) {
            // We have found a group_id where the requested key exists in the KeySpaceRange
            auto client_response = new proto::client::FindKeyRangeResponse();
            client_response->set_slave_group_id(group_id.id);
            _respond(endpoint_id, client_response);
            return -1;
          }
        }
      }
      // If no KeySpace has the desired key, then we search the NewKeySpaces
      // to see if there is one slave group that's in the progress of having that
      // key range assigned. If there is, then send the corresponding slaves a
      // NewKeySpaceSelected message.
      if (auto const& key_space = std::get_if<NewKeySpace>(&v)) {
        for (auto const& range : key_space->ranges) {
          if (within_range(range, message)) {
            // We have found a group_id where the requested key exists in the KeySpaceRange
            auto endpoints = _config_manager.get_endpoints(group_id);
            auto wrapper = build_new_key_space_selected_message(*key_space);
            _slave_connections.broadcast(endpoints, wrapper.SerializeAsString());
            return 100; // Repeatedly bombard the slaves until they have dealt with this message.
          }
        }
      }
    }

    // Assign the key-space range to the first
    
    return 100; // TODO make this into a constant
  });
}

} // namespace master
} // namespace uni
