#include "LogSyncer.h"

#include <algorithm>
#include <memory>
#include <tuple>
#include <vector>

#include <paxos/PaxosTypes.h>
#include <proto/paxos.pb.h>

namespace uni {
namespace slave {

using uni::async::TimerAsyncScheduler;
using uni::constants::Constants;
using uni::net::ConnectionsOut;
using uni::paxos::PaxosLog;
using uni::slave::FailureDetector;

LogSyncer::LogSyncer(
    Constants const& constants,
    ConnectionsOut& connections_out,
    TimerAsyncScheduler& timer_scheduler,
    PaxosLog& paxos_log,
    FailureDetector& failure_detector)
      : _constants(constants),
        _connections_out(connections_out),
        _timer_scheduler(timer_scheduler),
        _paxos_log(paxos_log),
        _failure_detector(failure_detector) {
  schedule_syncing();
}

void LogSyncer::schedule_syncing() {
  _timer_scheduler.schedule_repeated([this]() {
    auto message_wrapper = std::make_unique<proto::message::MessageWrapper>();
    auto slave_message = new proto::slave::SlaveMessage;
    auto sync_request = _inner::build_sync_request(_paxos_log.get_available_indices());
    slave_message->set_allocated_sync_request(sync_request);
    message_wrapper->set_allocated_slave_message(slave_message);

    // Send the sync_request to the leader.
    _connections_out.broadcast(message_wrapper->SerializeAsString());
  }, _constants.log_syncer_period);
}

void LogSyncer::handle_sync_request(uni::net::endpoint_id endpoint_id, proto::slave::SyncRequest request) {
  auto message_wrapper = std::make_unique<proto::message::MessageWrapper>();
  auto slave_message = new proto::slave::SlaveMessage;
  auto sync_response = _inner::build_sync_response(_paxos_log, &request);
  slave_message->set_allocated_sync_response(sync_response);
  message_wrapper->set_allocated_slave_message(slave_message);

  // Send the response to the sender.
  _connections_out.send(endpoint_id, message_wrapper->SerializeAsString());
}

// TODO: test right
void LogSyncer::handle_sync_response(proto::slave::SyncResponse response) {
  for (auto const& entry_with_index : response.missing_entries()) {
    _paxos_log.set_entry(entry_with_index.index(), entry_with_index.entry());
  }
}

namespace _inner {

// TODO: return a unique_ptr?
proto::slave::SyncRequest* build_sync_request(
  std::vector<uni::paxos::index_t> available_indices
) {
  auto sync_request = new proto::slave::SyncRequest;

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
    auto index_subarray = new proto::slave::SyncRequest_IndexSubArray;
    index_subarray->set_start(std::get<0>(subarray));
    index_subarray->set_end(std::get<1>(subarray));
    sync_request->mutable_missing_indices()->AddAllocated(index_subarray);
  }
  sync_request->set_last_index(*available_indices.begin());
  return sync_request;
}

proto::slave::SyncResponse* build_sync_response(
  uni::paxos::PaxosLog& paxos_log,
  proto::slave::SyncRequest* request
) {
  auto sync_response = new proto::slave::SyncResponse;

  // Handle the "holes" in the sender's PaxosLog.
  auto cur_last_index = paxos_log.next_available_index();
  for (auto const& index_subarray : request->missing_indices()) {
    auto start = index_subarray.start();
    auto end = index_subarray.end();
    if (start >= cur_last_index) {
      break;
    }
    end = std::min(cur_last_index, uni::paxos::index_t(end));
    for (auto i = start; i <= end; i++) {
      auto const entry = paxos_log.get_entry(i);
      if (entry) {
        auto entry_with_index = new proto::slave::SyncResponse_PaxosLogEntryWithIndex;
        entry_with_index->set_allocated_entry(new proto::paxos::PaxosLogEntry(entry.get()));
        entry_with_index->set_index(i);
        sync_response->mutable_missing_entries()->AddAllocated(entry_with_index);
      }
    }
  }

  // If the current node has PaxosLog entries that go beyond that of the sender's
  // then send these extra entries too.
  if (cur_last_index > request->last_index()) {
    for (auto i = request->last_index(); i < cur_last_index; i++) {
      auto const entry = paxos_log.get_entry(i);
      if (entry) {
        auto entry_with_index = new proto::slave::SyncResponse_PaxosLogEntryWithIndex;
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
