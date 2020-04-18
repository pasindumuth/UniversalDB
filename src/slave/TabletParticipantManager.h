#ifndef UNI_SLAVE_TABLETPARTICIPANTMANAGER_H
#define UNI_SLAVE_TABLETPARTICIPANTMANAGER_H

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/optional.hpp>

#include <common/common.h>
#include <net/Connections.h>
#include <net/IncomingMessage.h>
#include <server/KeySpaceRange.h>
#include <slave/functors.h>
#include <slave/TabletId.h>
#include <slave/TabletParticipant.h>

namespace uni {
namespace slave {

class TabletParticipantManager {
 public:
  struct KeySpaceSuffix {
    boost::optional<std::string> start_key;
    boost::optional<std::string> end_key;
  };

  using TPMapT = std::unordered_map<
    TabletId,
    uni::custom_unique_ptr<uni::slave::TabletParticipant>
  >;

  TabletParticipantManager(
    std::function<uni::custom_unique_ptr<uni::slave::TabletParticipant>(uni::slave::TabletId)> tp_provider,
    uni::net::Connections& client_connections,
    uni::slave::ClientRespond respond_callback);

  void handle(uni::net::IncomingMessage incoming_message);

  void handle_key_space_change(std::vector<uni::server::KeySpaceRange> const& ranges);

  TPMapT const& get_tps() const;

 private:
  std::function<uni::custom_unique_ptr<uni::slave::TabletParticipant>(uni::slave::TabletId)> _tp_provider;
  uni::net::Connections& _client_connections;
  uni::slave::ClientRespond _respond_callback;

  TPMapT _tp_map;

  void handle_client_request(
    proto::client::ClientRequest client_request,
    uni::net::IncomingMessage incoming_message);

  void handle_tablet_message(
    proto::tablet::TabletMessage tablet_message,
    uni::net::IncomingMessage incoming_message);
};

} // namespace slave
} // namespace uni


#endif // UNI_SLAVE_TABLETPARTICIPANTMANAGER_H
