#include "PaxosLog.h"

#include <sstream>

#include <assert/assert.h>
#include <logging/log.h>
#include <utils/stringutils.h>

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
  UNIVERSAL_ASSERT_MESSAGE(_available_indices.size() > 0, "Set of available indices should never be 0")
  auto const last_index = _available_indices.back();
  auto const it = std::find(_available_indices.begin(), _available_indices.end(), index);
  auto const first_available_index = next_available_index();
  if (index <= last_index && it == _available_indices.end()) {
    // A value for the index is already set.
    return;
  }
  if (index < last_index) {
    // index must be in _available_indices
    _available_indices.erase(it);
  } else if (index == last_index) {
    // index must be in _available_indices
    _available_indices.erase(it);
    _available_indices.push_back(index + 1);
  } else {
    // index must not be in _available_indices
    for (index_t i = last_index + 1; i < index; i++) {
      _available_indices.push_back(i);
    }
    _available_indices.push_back(index + 1);
  }
  _log.insert({ index, entry });
  auto next_first_available_index = next_available_index();
  if (first_available_index < next_first_available_index) {
    // This means that the we can fill out more of the derived
    // state. So invoke the callbacks for all new PaxosLogEntries
    for (int i = first_available_index; i < next_first_available_index; i++) {
      for (auto const& callback: _callbacks) {
        callback(_log[i]);
      }
    }
  }
}

void PaxosLog::add_callback(std::function<void(PaxosLogEntry)> callback) {
  _callbacks.push_back(callback);
}

index_t PaxosLog::next_available_index() const {
  UNIVERSAL_ASSERT_MESSAGE(_available_indices.size() > 0, "Set of available indices should never be 0")
  UNIVERSAL_ASSERT_MESSAGE(_log.find(*_available_indices.begin()) == _log.end(),
      "The index that is going to be returned as available must actually be available.")
  return *_available_indices.begin();
}

std::vector<index_t> PaxosLog::get_available_indices() const {
  return _available_indices;
}

std::unordered_map<index_t, proto::paxos::PaxosLogEntry const> PaxosLog::get_log() const {
  return _log;
}

std::string PaxosLog::debug_string() const {
  auto ss = std::stringstream();
  ss << "PaxosLog: {" << std::endl;
  for (auto const& [index, entry] : _log) {
    ss << "  " << "index: " << index << std::endl;
    ss << "  " << "value: {" << std::endl;
    ss << uni::utils::indent(entry.DebugString(), 4);
    ss << "  }" << std::endl;
  }
  ss << "}" << std::endl;
  return ss.str();
}

} // namespace paxos
} // namespace uni
