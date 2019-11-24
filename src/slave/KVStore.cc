#include "KVStore.h"

#include <assert/assert.h>

namespace uni {
namespace slave {

using proto::paxos::PaxosLogEntry;

void KVStore::write(std::string key, std::string value, timestamp_t timestamp) {
  auto it = _mvkvs.find(key);
  if (it == _mvkvs.end()) {
    _mvkvs.insert({key, {{{timestamp, value}}, timestamp}});
  } else {
    UNIVERSAL_ASSERT_MESSAGE(it->second.lat < timestamp, "Should never be writing to the past.")
    _mvkvs[key].versions.push_back({timestamp, value});
    _mvkvs[key].lat = timestamp;
  }
}

boost::optional<std::string> KVStore::read(std::string key, timestamp_t timestamp) {
  auto it = _mvkvs.find(key);
  if (it == _mvkvs.end()) {
    return boost::none;
  } else {
    // Perform binary search to find the version right before the timestamp.
    auto const& versions =  _mvkvs[key].versions;
    int start = 0;
    int end = versions.size();
    while (start < end) {
      int middle = (start + end) / 2;
      if (versions[middle].timestamp < timestamp) {
        start = middle + 1;
      } else {
        end = middle;
      }
    }
    if (start == 0) {
      // The timestamp provided was before the timestamp of any writes.
      return boost::none;
    } else {
      if (timestamp > _mvkvs[key].lat) {
        _mvkvs[key].lat = timestamp;
      }
      return versions[start - 1].value;
    }
  }
}

std::function<void(PaxosLogEntry)> KVStore::get_paxos_callback() {
  return [this](PaxosLogEntry entry) {
    switch (entry.type()) {
      case PaxosLogEntry::WRITE:
        write(entry.key(), entry.value(), entry.timestamp());
        break;
      case PaxosLogEntry::READ:
        read(entry.key(), entry.timestamp());
        break;
      default:
        // No-op
        break;
    }
  };
}

} // namespace slave
} // namespace uni
