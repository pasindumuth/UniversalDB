#include "PaxosLog.h"

#include <sstream>

#include <assert/assert.h>
#include <logging/log.h>

namespace uni {
namespace paxos {

using proto::paxos::PaxosLogEntry;

PaxosLog::PaxosLog() {
  _available_indices.push_back(0);
}

boost::optional<PaxosLogEntry const> PaxosLog::get_entry(index_t index) const {
  auto const it = _log.find(index);
  if (it != _log.end()) {
    return boost::optional<PaxosLogEntry const>(_log.at(index));
  } else {
    return boost::optional<PaxosLogEntry const>();
  }
}

void PaxosLog::set_entry(index_t index, PaxosLogEntry const entry) {
  UNIVERSAL_ASSERT_MESSAGE(_log.find(index) == _log.end(),
      "The index in the Paxos Log should never already be populated.")
  UNIVERSAL_ASSERT_MESSAGE(_available_indices.size() > 0, "Set of available indices should never be 0")
  auto const last_index = _available_indices.back();
  auto const it = std::find(_available_indices.begin(), _available_indices.end(), index);
  UNIVERSAL_ASSERT_MESSAGE(last_index < index || it != _available_indices.end(),
      "The index that the entry is being inserted should be a missing index or the index after the latest index")
  if (index < last_index) {
    _available_indices.erase(it);
  } else if (index == last_index) {
    _available_indices.erase(it);
    _available_indices.push_back(index + 1);
  } else {
    for (index_t i = last_index + 1; i < index; i++) {
      _available_indices.push_back(i);
    }
    _available_indices.push_back(index + 1);
  }
  _log.insert({ index, entry });
}

index_t PaxosLog::next_available_index() const {
  UNIVERSAL_ASSERT_MESSAGE(_available_indices.size() > 0, "Set of available indices should never be 0")
  UNIVERSAL_ASSERT_MESSAGE(_log.find(*_available_indices.begin()) == _log.end(),
      "The index that is going to be returned as available must actually be available.")
  return *_available_indices.begin();
}

void PaxosLog::debug_print() const {
  auto ss = std::stringstream();
  ss << "Printing PaxosLog:" << std::endl;
  for (auto const& [index, entry] : _log) {
    ss << "index: " << index << ", entry: " << entry.SerializeAsString() << std::endl;
  }
  ss << "End of PaxosLog" << std::endl;
  LOG(uni::logging::INFO, ss.str())
}

std::unordered_map<index_t, proto::paxos::PaxosLogEntry const> PaxosLog::get_log() const {
  return _log;
}

} // namespace paxos
} // namespace uni
