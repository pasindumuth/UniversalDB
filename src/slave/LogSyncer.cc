#include "LogSyncer.h"

#include <algorithm>
#include <memory>
#include <tuple>
#include <vector>

#include <paxos/PaxosTypes.h>
#include <proto/paxos.pb.h>

namespace uni {
namespace slave {

LogSyncer::LogSyncer(
    uni::constants::Constants const& constants,
    uni::net::Connections& connections,
    uni::async::TimerAsyncScheduler& timer_scheduler,
    uni::paxos::PaxosLog& paxos_log,
    uni::slave::FailureDetector& failure_detector,
    std::function<proto::message::MessageWrapper(proto::sync::SyncMessage*)> sync_message_to_wrapper)
      : _constants(constants),
        _connections(connections),
        _timer_scheduler(timer_scheduler),
        _paxos_log(paxos_log),
        _failure_detector(failure_detector),
        _sync_message_to_wrapper(sync_message_to_wrapper) {
  schedule_syncing();
}

void LogSyncer::schedule_syncing() {
  _timer_scheduler.schedule_repeated([this]() {
    auto sync_message = new proto::sync::SyncMessage;
    sync_message->set_allocated_sync_request(
      _inner::build_sync_request(_paxos_log.get_available_indices()));
    auto message_wrapper = _sync_message_to_wrapper(sync_message);
    // TODO only send the sync_request to the leader.
    _connections.broadcast(message_wrapper.SerializeAsString());
  }, _constants.log_syncer_period);
}

void LogSyncer::handle_sync_request(uni::net::endpoint_id endpoint_id, proto::sync::SyncRequest request) {
  auto sync_message = new proto::sync::SyncMessage;
  sync_message->set_allocated_sync_response(
    _inner::build_sync_response(_paxos_log, request));
  auto message_wrapper = _sync_message_to_wrapper(sync_message);
  // Send the response to the sender.
  _connections.send(endpoint_id, message_wrapper.SerializeAsString());
}

// TODO: test right
void LogSyncer::handle_sync_response(proto::sync::SyncResponse response) {
  for (auto const& entry_with_index : response.missing_entries()) {
    _paxos_log.set_entry(entry_with_index.index(), entry_with_index.entry());
  }
}

namespace _inner {

proto::sync::SyncRequest* build_sync_request(
  std::vector<uni::paxos::index_t> available_indices
) {
  auto sync_request = new proto::sync::SyncRequest;

  // Process available indices. Split them into sub arrays
  auto available_index_subarrays = std::vector<std::tuple<uni::paxos::index_t, uni::paxos::index_t>>();
  auto cur_subarray = std::tuple<uni::paxos::index_t, uni::paxos::index_t>(available_indices[0], available_indices[0]);
  for (auto i = 1; i < available_indices.size(); i++) {
    if (available_indices[i-1] + 1 == available_indices[i]) {
      std::get<1>(cur_subarray)++;
    } else {
      available_index_subarrays.push_back(cur_subarray);
      cur_subarray = std::tuple<uni::paxos::index_t, uni::paxos::index_t>(available_indices[i], available_indices[i]);
    }
  }
  available_index_subarrays.push_back(cur_subarray);
  for (auto const& subarray : available_index_subarrays) {
    auto index_subarray = new proto::sync::SyncRequest_IndexSubArray;
    index_subarray->set_start(std::get<0>(subarray));
    index_subarray->set_end(std::get<1>(subarray));
    sync_request->mutable_missing_indices()->AddAllocated(index_subarray);
  }
  sync_request->set_last_index(available_indices.back());
  return sync_request;
}

proto::sync::SyncResponse* build_sync_response(
  uni::paxos::PaxosLog& paxos_log,
  proto::sync::SyncRequest& request
) {
  auto sync_response = new proto::sync::SyncResponse;

  // Handle the "holes" in the sender's PaxosLog.
  auto cur_last_index = paxos_log.next_available_index();
  for (auto const& index_subarray : request.missing_indices()) {
    auto start = index_subarray.start();
    auto end = index_subarray.end();
    if (start >= cur_last_index) {
      break;
    }
    end = std::min(cur_last_index, uni::paxos::index_t(end));
    for (auto i = start; i <= end; i++) {
      auto const entry = paxos_log.get_entry(i);
      if (entry) {
        auto entry_with_index = new proto::sync::SyncResponse_PaxosLogEntryWithIndex;
        entry_with_index->set_allocated_entry(new proto::paxos::PaxosLogEntry(entry.get()));
        entry_with_index->set_index(i);
        sync_response->mutable_missing_entries()->AddAllocated(entry_with_index);
      }
    }
  }

  // If the current node has PaxosLog entries that go beyond that of the sender's
  // then send these extra entries too.
  if (cur_last_index > request.last_index()) {
    // We add 1 below, since the entry at request.last_index() should
    // have been added in the last loop.
    for (auto i = request.last_index() + 1; i < cur_last_index; i++) {
      auto const entry = paxos_log.get_entry(i);
      if (entry) {
        auto entry_with_index = new proto::sync::SyncResponse_PaxosLogEntryWithIndex;
        entry_with_index->set_allocated_entry(new proto::paxos::PaxosLogEntry(entry.get()));
        entry_with_index->set_index(i);
        sync_response->mutable_missing_entries()->AddAllocated(entry_with_index);
      }
    }
  }

  return sync_response;
}

} // namespace _inner
} // namespace slave
} // namespace uni
