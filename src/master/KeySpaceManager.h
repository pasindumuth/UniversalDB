#ifndef UNI_MASTER_KEYSPACEMANAGER_H
#define UNI_MASTER_KEYSPACEMANAGER_H

#include <unordered_map>
#include <variant>
#include <vector>

#include <async/AsyncQueue.h>
#include <common/common.h>
#include <master/functors.h>
#include <master/GroupConfigManager.h>
#include <net/Connections.h>
#include <paxos/MultiPaxosHandler.h>
#include <proto/client.pb.h>
#include <proto/message.pb.h>
#include <proto/slave.pb.h>
#include <server/KeySpaceRange.h>
#include <server/SlaveGroupId.h>

namespace uni {
namespace master {

class KeySpaceManager {
 public:
  static int const RETRY_LIMIT;
  static int const WAIT_FOR_COMMIT_INSERTED;
  static int const WAIT_FOR_COMMIT;
  static int const WAIT_FOR_FREE;
  static int const WAIT_FOR_NEW_KEY_SPACE;

  KeySpaceManager(
    uni::async::AsyncQueue& async_queue,
    std::function<uni::async::AsyncQueue()>& async_queue_provider,
    uni::master::GroupConfigManager& config_manager,
    uni::net::Connections& slave_connections,
    uni::paxos::MultiPaxosHandler& multipaxos_handler,
    uni::paxos::PaxosLog& paxos_log,
    uni::master::SendFindKeyRangeResponse respond);

  void set_first_config(uni::server::SlaveGroupId group_id);

  void handle_find_key(
    uni::net::EndpointId endpoint_id,
    proto::client::FindKeyRangeRequest const& message);

  void handle_key_space_changed(
    uni::net::EndpointId endpoint_id,
    proto::slave::KeySpaceChanged const& message);

 private:
  uni::async::AsyncQueue& _async_queue;
  uni::master::GroupConfigManager& _config_manager;
  uni::net::Connections& _slave_connections;
  uni::paxos::MultiPaxosHandler& _multipaxos_handler;
  uni::paxos::PaxosLog& _paxos_log;
  uni::master::SendFindKeyRangeResponse _respond;

  uni::async::AsyncQueue _local_async_queue;

  struct KeySpace {
    std::vector<uni::server::KeySpaceRange> ranges;
    uint32_t generation;
  };

  struct NewKeySpace {
    std::vector<uni::server::KeySpaceRange> ranges;
    std::vector<uni::server::KeySpaceRange> new_ranges;
    uint32_t generation;
  };

  std::unordered_map<uni::server::SlaveGroupId, std::variant<KeySpace, NewKeySpace>> _slave_group_ranges;

  proto::message::MessageWrapper build_new_key_space_selected_message(
    NewKeySpace const& key_space);

  proto::paxos::master::NewKeySpaceSelected* build_new_key_space_selected_paxos(
    uni::server::SlaveGroupId group_id,
    std::vector<uni::server::KeySpaceRange> const& new_ranges,
    uint32_t generation);
};

} // namespace master
} // namespace uni


#endif // UNI_MASTER_KEYSPACEMANAGER_H
