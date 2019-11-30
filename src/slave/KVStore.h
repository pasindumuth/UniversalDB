#ifndef UNI_SLAVE_KVSTORE_H
#define UNI_SLAVE_KVSTORE_H

#include <functional>
#include <string>
#include <map>
#include <vector>

#include <boost/optional.hpp>

#include <common/types.h>
#include <proto/paxos.pb.h>

namespace uni {
namespace slave {

class KVStore {
 public:
  struct MultiVersionEntry {
    struct VersionEntry {
      timestamp_t timestamp;
      std::string value;
    };

    std::vector<VersionEntry> versions; // versions
    timestamp_t lat; // Last access timestamp (write or read)
  };
  
  void write(std::string key, std::string value, timestamp_t timestamp);

  boost::optional<std::string> read(std::string key, timestamp_t timestamp);

  std::function<void(proto::paxos::PaxosLogEntry)> get_paxos_callback();

  std::string debug_string() const;

 private:
  std::map<std::string, MultiVersionEntry> _mvkvs;
};

} // namespace slave
} // namespace uni


#endif // UNI_SLAVE_KVSTORE_H
