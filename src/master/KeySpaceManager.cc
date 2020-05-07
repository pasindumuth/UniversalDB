#include "KeySpaceManager.h"

#include <boost/optional.hpp>

#include <assert/assert.h>
#include <common/common.h>
#include <paxos/PaxosTypes.h>
#include <proto/paxos.pb.h>
#include <utils/pbutil.h>

namespace uni {
namespace master {

int const KeySpaceManager::RETRY_LIMIT = 3;
int const KeySpaceManager::WAIT_FOR_COMMIT_INSERTED = 100;
int const KeySpaceManager::WAIT_FOR_COMMIT = 100;
int const KeySpaceManager::WAIT_FOR_FREE = 100;
int const KeySpaceManager::WAIT_FOR_NEW_KEY_SPACE = 100;

KeySpaceManager::KeySpaceManager(
  uni::async::AsyncQueue& async_queue,
  std::function<uni::async::AsyncQueue()>& async_queue_provider,
  uni::master::GroupConfigManager& config_manager,
  uni::net::Connections& slave_connections,
  uni::paxos::MultiPaxosHandler& multipaxos_handler,
  uni::paxos::PaxosLog& paxos_log,
  uni::master::SendFindKeyRangeResponse respond)
  : _async_queue(async_queue),
    _config_manager(config_manager),
    _slave_connections(slave_connections),
    _multipaxos_handler(multipaxos_handler),
    _paxos_log(paxos_log),
    _respond(respond),
    _local_async_queue(async_queue_provider())
{
  paxos_log.add_callback(
    proto::paxos::PaxosLogEntry::EntryContentCase::kKeySpaceSelected,
    [this](uni::paxos::index_t index, proto::paxos::PaxosLogEntry entry) {
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
        new_key_space.new_ranges.push_back(uni::server::convert(range));
      }

      _slave_group_ranges[group_id] = new_key_space;

      auto endpoints = _config_manager.get_endpoints(group_id);
      auto wrapper = build_new_key_space_selected_message(new_key_space);
      // TODO we should be broadcasting this message to all slaves.
      _slave_connections.broadcast({endpoints[0]}, wrapper.SerializeAsString());
    });

  paxos_log.add_callback(
    proto::paxos::PaxosLogEntry::EntryContentCase::kKeySpaceCommit,
    [this](uni::paxos::index_t index, proto::paxos::PaxosLogEntry entry) {
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
        new_key_space.new_ranges,
        new_key_space.generation
      };
    });
}

void KeySpaceManager::set_first_config(uni::server::SlaveGroupId group_id) {
  _slave_group_ranges.insert({group_id, KeySpace{{}, 0}});
}

void KeySpaceManager::handle_find_key(
  uni::net::EndpointId endpoint_id,
  proto::message::client::FindKeyRangeRequest const& message
) {
  auto retry_count = std::make_shared<int>(0);
  _async_queue.add_task([this, retry_count, message, endpoint_id]() {
    // Search the key space ranges to see if there is a slave group
    // that has the desired key range. If it does, reply to the client.
    for (auto const& [group_id, v] : _slave_group_ranges) {
      if (auto const& key_space = std::get_if<KeySpace>(&v)) {
        for (auto const& range : key_space->ranges) {
          if (uni::server::within_range(range, message)) {
            // We have found a group_id where the requested key exists in the KeySpaceRange
            auto client_response = new proto::message::client::FindKeyRangeResponse();
            client_response->set_slave_group_id(group_id.id);
            _respond(endpoint_id, client_response);
            return uni::async::AsyncQueue::TERMINATE;
          }
        }
      }
      // If no KeySpace has the desired key, then we search the NewKeySpaces
      // to see if there is one slave group that's in the progress of having that
      // key range assigned. If there is, then send the corresponding slaves a
      // NewKeySpaceSelected message.
      if (auto const& key_space = std::get_if<NewKeySpace>(&v)) {
        for (auto const& range : key_space->new_ranges) {
          if (uni::server::within_range(range, message)) {
            // We have found a group_id where the requested key exists in the KeySpaceRange
            auto endpoints = _config_manager.get_endpoints(group_id);
            auto wrapper = build_new_key_space_selected_message(*key_space);
            // TODO we should be broadcasting this message to all slaves.
            _slave_connections.broadcast({endpoints[0]}, wrapper.SerializeAsString());
            return WAIT_FOR_COMMIT_INSERTED; // Repeatedly bombard the slaves until they have dealt with this message.
          }
        }
      }
    }

    if (*retry_count == RETRY_LIMIT) {
      auto client_response = new proto::message::client::FindKeyRangeResponse();
      client_response->set_error_code(proto::message::client::Code::ERROR);
      _respond(endpoint_id, client_response);
      return uni::async::AsyncQueue::TERMINATE;
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
      return WAIT_FOR_FREE;
    }

    auto const& key_space = std::get<KeySpace>(it->second);
    auto new_ranges = key_space.ranges;
    new_ranges.push_back({message.database_id(), message.table_id(), boost::none, boost::none});

    auto log_entry = proto::paxos::PaxosLogEntry();
    auto new_key_space_message = build_new_key_space_selected_paxos(it->first, new_ranges, key_space.generation + 1);
    log_entry.set_allocated_key_space_selected(new_key_space_message);
    _multipaxos_handler.propose(log_entry);
    *retry_count += 1;
    return WAIT_FOR_NEW_KEY_SPACE;
  });
}

void KeySpaceManager::handle_key_space_changed(
  uni::net::EndpointId endpoint_id,
  proto::message::slave::KeySpaceChanged const& message
) {
  // We don't need to worry about retrying here, the one branch where
  // the job lives on will eventually evaluate to false, ending the job automatically.
  _local_async_queue.add_task([this, message, endpoint_id]() {
    if (auto group_id = _config_manager.get_group_id(endpoint_id)) {
      auto const& state = _slave_group_ranges[group_id.get()];
      if (auto const& new_key_space = std::get_if<NewKeySpace>(&state)) {
        if (new_key_space->generation == message.generation()) {
          auto log_entry = proto::paxos::PaxosLogEntry();
          auto commit = new proto::paxos::master::KeySpaceCommit;
          commit->set_generation(message.generation());
          commit->set_slave_group_id(group_id.get().id);
          log_entry.set_allocated_key_space_commit(commit);
          _multipaxos_handler.propose(log_entry);
          return WAIT_FOR_COMMIT;
        }
      }
    }
    // If we get here, then either the group_id doesn't exist, the current state
    // is KeySpace (instead of NewKeySpace), or the generation numbers are off.
    // Either way, we can't complete this step, so just terminate the job.
    return uni::async::AsyncQueue::TERMINATE;
  });
}

proto::message::MessageWrapper KeySpaceManager::build_new_key_space_selected_message(NewKeySpace const& key_space) {
  auto message_wrapper = proto::message::MessageWrapper();
  auto master_message = new proto::message::master::MasterMessage();
  auto new_key_space_message = new proto::message::master::NewKeySpaceSelected();
  for (auto const& range : key_space.new_ranges) {
    uni::server::build_range(new_key_space_message->add_new_ranges(), range);
  }
  new_key_space_message->set_generation(key_space.generation);
  master_message->set_allocated_key_space_selected(new_key_space_message);
  message_wrapper.set_allocated_master_message(master_message);
  return message_wrapper;
}

proto::paxos::master::NewKeySpaceSelected* KeySpaceManager::build_new_key_space_selected_paxos(
  uni::server::SlaveGroupId group_id,
  std::vector<uni::server::KeySpaceRange> const& new_ranges,
  uint32_t generation
) {
  auto new_key_space_message = new proto::paxos::master::NewKeySpaceSelected();
  for (auto const& range : new_ranges) {
    uni::server::build_range(new_key_space_message->add_new_ranges(), range);
  }
  new_key_space_message->set_generation(generation);
  new_key_space_message->set_slave_group_id(group_id.id);
  return new_key_space_message;
}

} // namespace master
} // namespace uni
