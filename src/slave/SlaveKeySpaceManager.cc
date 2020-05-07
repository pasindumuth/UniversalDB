#include "SlaveKeySpaceManager.h"

#include <assert/assert.h>
#include <proto/message.pb.h>
#include <proto/paxos_slave.pb.h>
#include <proto/message_slave.pb.h>

#include <utils/pbutil.h>

namespace uni {
namespace slave {

int const SlaveKeySpaceManager::WAIT_FOR_PAXOS = 100;

SlaveKeySpaceManager::SlaveKeySpaceManager(
  uni::async::AsyncQueue& async_queue,
  uni::net::Connections& master_connections,
  uni::paxos::MultiPaxosHandler& multipaxos_handler,
  uni::paxos::PaxosLog& paxos_log,
  uni::slave::TabletParticipantManager& participant_manager)
  : _async_queue(async_queue),
    _master_connections(master_connections),
    _multipaxos_handler(multipaxos_handler),
    _paxos_log(paxos_log),
    _ranges{
      std::vector<uni::server::KeySpaceRange>(),
      0
    },
    _participant_manager(participant_manager)
{
  paxos_log.add_callback(
    proto::paxos::PaxosLogEntry::EntryContentCase::kKeySpaceChanged,
    [this](uni::paxos::index_t index, proto::paxos::PaxosLogEntry entry) {
      auto const& changed_message = entry.key_space_changed();
      UNIVERSAL_ASSERT_MESSAGE(changed_message.generation() == _ranges.generation + 1,
        "The generation of the KeySpaceChanged message should be 1 greater than the current.")

      _ranges = {
        std::vector<uni::server::KeySpaceRange>(),
        changed_message.generation()
      };

      for (auto const& range: changed_message.new_ranges()) {
        _ranges.ranges.push_back(uni::server::convert(range));
      }

      // Update the Tablet Participants with any new ranges that have been added.
      _participant_manager.handle_key_space_change(_ranges.ranges);

      // Reply back to the DMs.
      auto message_wrapper = proto::message::MessageWrapper();
      auto slave_message = new proto::message::slave::SlaveMessage;
      auto key_space_changed = new proto::message::slave::KeySpaceChanged;
      key_space_changed->set_generation(_ranges.generation);
      slave_message->set_allocated_key_space_changed(key_space_changed);
      message_wrapper.set_allocated_slave_message(slave_message);
      auto endpoints = _master_connections.get_all_endpoints();
      _master_connections.broadcast({endpoints[0]}, message_wrapper.SerializeAsString());
    });
}

void SlaveKeySpaceManager::handle_key_space_change(
  uni::net::EndpointId endpoint_id,
  proto::message::master::NewKeySpaceSelected const& message
) {
  // We don't need to worry about retrying here, because the _ranges.generation
  // here will eventually get to a point that executes a branch that terminates the job.
  _async_queue.add_task([this, message, endpoint_id]() {
    if (_ranges.generation + 1 < message.generation()) {
      // This means that the slave is behind. In this case, we want to break
      // out and do nothing. This node will eventually catch up to a point where
      // this if condition evaluates to false.
      return uni::async::AsyncQueue::TERMINATE;
    } else if (_ranges.generation + 1 == message.generation()) {
      // The state of this is node is just right to try proposing the next generation
      // key space ranges.
      auto log_entry = proto::paxos::PaxosLogEntry();
      auto paxos_key_space_changed = new proto::paxos::slave::KeySpaceChanged;
      for (auto const& range : message.new_ranges()) {
        auto range_to_add = paxos_key_space_changed->add_new_ranges();
        range_to_add->set_database_id(range.database_id());
        range_to_add->set_table_id(range.table_id());
        if (range.has_start_key()) {
          range_to_add->set_allocated_start_key(uni::utils::pb::string(range.start_key()));
        }
        if (range.has_end_key()) {
          range_to_add->set_allocated_end_key(uni::utils::pb::string(range.end_key()));
        }
      }
      paxos_key_space_changed->set_generation(message.generation());
      log_entry.set_allocated_key_space_changed(paxos_key_space_changed);
      _multipaxos_handler.propose(log_entry);
      return WAIT_FOR_PAXOS;
    } else if (_ranges.generation == message.generation()) {
      // This means that the slave nodes have updated the key space ranges to that
      // requested by this incoming message, so just reply to the master with a 
      // commit message.
      auto message_wrapper = proto::message::MessageWrapper();
      auto slave_message = new proto::message::slave::SlaveMessage;
      auto key_space_changed = new proto::message::slave::KeySpaceChanged;
      key_space_changed->set_generation(message.generation());
      slave_message->set_allocated_key_space_changed(key_space_changed);
      message_wrapper.set_allocated_slave_message(slave_message);
      _master_connections.broadcast({endpoint_id}, message_wrapper.SerializeAsString());
      return uni::async::AsyncQueue::TERMINATE;
    } else {
      // This means that the sent message was from a datamaster that's behind. Just
      // ignore it, since the datamaster will catch up on it's own.
      return uni::async::AsyncQueue::TERMINATE;
    }
  });
}

} // namespace slave
} // namespace uni
