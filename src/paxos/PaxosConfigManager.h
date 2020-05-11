#ifndef UNI_PAXOS_PAXOSCONFIGMANAGER_H
#define UNI_PAXOS_PAXOSCONFIGMANAGER_H

#include <unordered_map>
#include <vector>

#include <common/common.h>
#include <net/EndpointId.h>
#include <paxos/PaxosLog.h>
#include <paxos/PaxosTypes.h>

namespace uni {
namespace paxos {

class PaxosConfigManager {
 public:

  /**
   * @brief The @p index and @p slaves form the first entry for the _config_timeline field.
   */
  PaxosConfigManager(
    uni::paxos::index_t index,
    std::vector<uni::net::EndpointId> slaves,
    uni::paxos::PaxosLog& paxos_log);

  /**
   * @brief Gets the config at index @p index . Recall that according to the
   * the MultiPaxos Transition System, this is the config with the highest
   * index such that it's <= @p index .
   */
  std::vector<uni::net::EndpointId> config(uni::paxos::index_t index) const;

  /**
   * @brief Get the latest configuration.
   */
  std::vector<uni::net::EndpointId> latest_config() const;

 private:
  struct IndexAndConfig {
    uni::paxos::index_t index;
    std::vector<uni::net::EndpointId> slaves;
  };

  // The index of the entries in this vector must always ascending (since a new config
  // can only be selected once it's inserted into the paxos log, and since we never
  // insert backwards in time).
  std::vector<IndexAndConfig> _config_timeline;
};

} // namespace paxos
} // namespace uni


#endif // UNI_PAXOS_PAXOSCONFIGMANAGER_H
