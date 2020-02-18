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
    // Perform binary search to find the version such that all version (strictly)
    // before have <= timestamp, and all versions (non-strictly) above have > timestamp.
    auto const& versions =  _mvkvs[key].versions;
    auto start = 0;
    auto end = versions.size();
    while (start < end) {
      auto middle = (start + end) / 2;
      if (versions[middle].timestamp <= timestamp) {
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

boost::optional<timestamp_t> KVStore::read_lat(std::string key) {
  auto it = _mvkvs.find(key);
  if (it == _mvkvs.end()) {
    return boost::none;
  } else {
    return it->second.lat;
  }
}

std::function<void(uni::paxos::index_t, PaxosLogEntry)> KVStore::get_paxos_callback() {
  return [this](uni::paxos::index_t index, PaxosLogEntry entry) {
    switch (entry.type()) {
      case PaxosLogEntry::WRITE:
        write(entry.key().value(), entry.value().value(), entry.timestamp().value());
        break;
      case PaxosLogEntry::READ:
        read(entry.key().value(), entry.timestamp().value());
        break;
      default:
        // No-op
        break;
    }
  };
}

std::string KVStore::debug_string() const {
  auto ss = std::stringstream();
  ss << "KVStore: {" << std::endl;
  for (auto const& [key, entry] : _mvkvs) {
    ss << "  " << "key: " << key << std::endl;
    ss << "  " << "value: {" << std::endl;
    ss << "    " << "lat: " << entry.lat << std::endl;
    ss << "    " << "versions: [" << std::endl;
    for (auto const& version: entry.versions) {
      ss << "      " << "{ value: " << version.value << ", timestamp: " << version.timestamp << " }" << std::endl;
    }
    ss << "    " << "]" << std::endl;
    ss << "  " << "}" << std::endl;
  }
  ss << "}" << std::endl;
  return ss.str();
}

} // namespace slave
} // namespace uni
