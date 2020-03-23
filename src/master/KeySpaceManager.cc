#include "KeySpaceManager.h"

#include <boost/optional.hpp>

#include <assert/assert.h>
#include <common/common.h>
#include <paxos/PaxosTypes.h>
#include <proto/paxos.pb.h>
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
    _respond(respond)
{
  paxos_log.add_callback([this](uni::paxos::index_t index, proto::paxos::PaxosLogEntry entry){
    if (entry.has_key_space_selected()) {
      auto const& key_space_selected = entry.key_space_selected();
      auto const& group_id = uni::server::SlaveGroupId{key_space_selected.slave_group_id()};
      UNIVERSAL_ASSERT_MESSAGE(_slave_group_ranges.find(group_id) != _slave_group_ranges.end(),
        "There should already be a slave group before a NewKeySpaceSelected message is inserted for that group");
      if (std::get_if<NewKeySpace>(&_slave_group_ranges[group_id])) {
        UNIVERSAL_TERMINATE(
          "When a NewKeySpaceSelected paxos message is inserted, the key space state should be KeySpace, not NewKeySpace");
      }
      auto key_space = std::get<KeySpace>(_slave_group_ranges[group_id]);
      UNIVERSAL_ASSERT_MESSAGE(key_space.generation + 1 == key_space_selected.generation(),
        "The generation of the NewKeySpaceSelected should be one greater than the old");

      auto new_key_space = NewKeySpace{
        key_space.ranges,
        std::vector<uni::server::KeySpaceRange>(),
        key_space_selected.generation()
      };

      for (auto const& range: key_space_selected.new_ranges()) {
        new_key_space.new_ranges.push_back(extract_range(range));
      }

      _slave_group_ranges[group_id] = new_key_space;
    }

    if (entry.has_key_space_commit()) {
      auto const& key_space_commit = entry.key_space_commit();
      auto const& group_id = uni::server::SlaveGroupId{key_space_commit.slave_group_id()};
      UNIVERSAL_ASSERT_MESSAGE(_slave_group_ranges.find(group_id) != _slave_group_ranges.end(),
        "There should already be a slave group before a KeySpaceCommit message is inserted for that group");
      if (std::get_if<KeySpace>(&_slave_group_ranges[group_id])) {
        UNIVERSAL_TERMINATE(
          "When a KeySpaceCommit paxos message is inserted, the key space state should be NewKeySpace, not KeySpace");
      }
      auto new_key_space = std::get<NewKeySpace>(_slave_group_ranges[group_id]);
      UNIVERSAL_ASSERT_MESSAGE(new_key_space.generation == key_space_commit.generation(),
        "The generation of the KeySpaceCommit should be equals to that of the current new key space");
      
      _slave_group_ranges[group_id] = KeySpace{
        new_key_space.ranges,
        new_key_space.generation
      };
    }
  });
}

void KeySpaceManager::set_first_config(uni::server::SlaveGroupId group_id) {
  _slave_group_ranges.insert({group_id, KeySpace{{}, 0}});
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

    // Try to assign the key-space range to the first group by proposing this change with paxos.

    // TODO: for now, we assume there is at least one paxos group. In the future, if there is no paxos
    // group, then we must use GroupConfigManager to create one.

    auto it = _slave_group_ranges.begin();
    while (it != _slave_group_ranges.end() && std::get_if<NewKeySpace>(&it->second)) {
      it++;
    }

    if (it == _slave_group_ranges.end()) {
      // Try again in a bit. Soon, there should be a slave group
      // whose key space gets back to the steady state (KeySpace).
      return 100;
    }

    auto const& key_space = std::get<KeySpace>(it->second);
    auto new_ranges = key_space.ranges;
    new_ranges.push_back({message.database_id(), message.table_id(), boost::none, boost::none});
    auto new_key_space = NewKeySpace{
      key_space.ranges,
      new_ranges,
      key_space.generation + 1
    };

    auto log_entry = proto::paxos::PaxosLogEntry();
    auto new_key_space_message = build_new_key_space_selected_paxos(it->first, new_key_space);
    log_entry.set_allocated_key_space_selected(new_key_space_message);
    _multipaxos_handler.propose(log_entry);
    return 100; // TODO make this into a constant
  });
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

void KeySpaceManager::build_range(
  proto::common::KeySpaceRange *const proto_range,
  uni::server::KeySpaceRange const& range
) {
  proto_range->set_database_id(range.database_id);
  proto_range->set_table_id(range.table_id);
  if (range.start_key) proto_range->set_allocated_start_key(uni::utils::pb::string(range.start_key.get()));
  if (range.end_key) proto_range->set_allocated_end_key(uni::utils::pb::string(range.end_key.get()));
}

proto::message::MessageWrapper KeySpaceManager::build_new_key_space_selected_message(NewKeySpace const& key_space) {
  auto message_wrapper = proto::message::MessageWrapper();
  auto master_message = new proto::master::MasterMessage();
  auto new_key_space_message = new proto::master::NewKeySpaceSelected();
  for (auto const& range : key_space.new_ranges) {
    build_range(new_key_space_message->add_new_ranges(), range);
  }
  new_key_space_message->set_generation(key_space.generation);
  master_message->set_allocated_key_space_selected(new_key_space_message);
  message_wrapper.set_allocated_master_message(master_message);
  return message_wrapper;
}
proto::paxos::master::NewKeySpaceSelected* KeySpaceManager::build_new_key_space_selected_paxos(
  uni::server::SlaveGroupId group_id,
  NewKeySpace const& key_space
) {
  auto new_key_space_message = new proto::paxos::master::NewKeySpaceSelected();
  for (auto const& range : key_space.new_ranges) {
    build_range(new_key_space_message->add_new_ranges(), range);
  }
  new_key_space_message->set_generation(key_space.generation);
  new_key_space_message->set_slave_group_id(group_id.id);
  return new_key_space_message;
}

uni::server::KeySpaceRange KeySpaceManager::extract_range(proto::common::KeySpaceRange const& proto_range) {
  return {
    proto_range.database_id(),
    proto_range.table_id(),
    proto_range.has_start_key() ? proto_range.start_key().value() : boost::optional<std::string>(),
    proto_range.has_end_key() ? proto_range.end_key().value() : boost::optional<std::string>()
  };
}

} // namespace master
} // namespace uni
