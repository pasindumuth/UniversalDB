#include "PaxosConfigManager.h"

#include <assert/assert.h>
#include <proto/paxos.pb.h>

namespace uni {
namespace paxos {

PaxosConfigManager::PaxosConfigManager(
  uni::paxos::index_t index,
  std::vector<uni::net::EndpointId> slaves,
  uni::paxos::PaxosLog& paxos_log)
  : _config_timeline({{index, slaves}})
{
  paxos_log.add_callback(
    proto::paxos::PaxosLogEntry::EntryContentCase::kReconfigStarted,
    [this](uni::paxos::index_t index, proto::paxos::PaxosLogEntry entry) {
      UNIVERSAL_ASSERT_MESSAGE(index > _config_timeline.back().index,
        "Index should always be ascending.")
      auto const& reconfig_message = entry.reconfig_started();
      auto new_slaves = std::vector<uni::net::EndpointId>{};
      for (auto slave : reconfig_message.slaves()) {
        new_slaves.push_back({slave.url(), 0});
      }
      // The new config should be applied starting from the next index.
      _config_timeline.push_back({index + 1, new_slaves});
    });
}

std::vector<uni::net::EndpointId> PaxosConfigManager::config(uni::paxos::index_t index) const {
  int i = _config_timeline.size() - 1;
  for (; i >= 0 && _config_timeline[i].index > index; i--);
  UNIVERSAL_ASSERT_MESSAGE(i >= 0,
    "Config should only be requested for an index governed by this Paxos timeline.")
  return _config_timeline[i].slaves;
}

std::vector<uni::net::EndpointId> PaxosConfigManager::latest_config() const {
  return _config_timeline.back().slaves;
}

} // namespace paxos
} // namespace uni
