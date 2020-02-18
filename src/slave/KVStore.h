#ifndef UNI_SLAVE_KVSTORE_H
#define UNI_SLAVE_KVSTORE_H

#include <functional>
#include <string>
#include <map>
#include <vector>

#include <boost/optional.hpp>

#include <common/common.h>
#include <paxos/PaxosTypes.h>
#include <proto/paxos.pb.h>

namespace uni {
namespace slave {

/**
 * @brief A representation of a multi-version key-value store (MVKVS).
 * 
 * Each key, a string value, maps to a list of (value, timestamp) pairs. These pairs are
 * called versions. Version lists are modified as append only, with each subsequent version
 * has a greater timestamp. This MVKVS enables snapshot reads; if a key exists, a read of the
 * kay at a certain time will always return the same value.
 * 
 * To read from a MVKVS, a client passes in a key and timestamp. The version with timestamp
 * before or at the provided timestamp is returned. If there is no such key, or no such version,
 * then an empty optional is returned.
 * 
 * To write to a MVKVS, a client passes in a key, value, and timestamp. The timestamp must be
 * strictly greater than the last timestamp that the key was accessed. This includes after a read.
 * So for example, if the last timestamp for a key happened from a write at time 10, and then a
 * read happened for that key at timestamp 13, then a subsequent write cannot write to the key at
 * any time <= 13. If this was possible, a subsequent read at timestamp 13 would return a different
 * value, defeating the ability to do snapshot reads.
 * 
 * Note that last access times apply to keys, not the whole MVKVS. That is, a key k1 can have 
 * last access time 10, and another key k2 can have last access time 5. Thus, a write to k2
 * can happen at time 8 without any worry.
 * 
 * All data is stored in memory.
 */ 
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
  
  /**
   * @brief Write a new version to the given key
   * 
   * If the key exists, then the timestamp must be greater than the last access time
   * of the key.
   */ 
  void write(std::string key, std::string value, timestamp_t timestamp);

  /**
   * @brief Read the value of a key at the given timestamp
   * 
   * If the key doesn't exist, returns an empty optiona. If it does exist, then the version whose
   * timestamp is <= than the @p timestamp is returned, and it's value returned. If no such
   * version exists (the @p timestamp is earlier than the earlier version), then an empty optiona
   * is returned.
   */
  boost::optional<std::string> read(std::string key, timestamp_t timestamp);

  /**
   * @brief Reads the last access time (lat) of the given key if it exist.
   */
  boost::optional<timestamp_t> read_lat(std::string key);

  /**
   * @brief Get a callback that can be registered with a uni::paxos::PaxosLog
   * 
   * This callback is used in the uni::paxos::PaxosLog to update the KVStore when
   * a proto::paxos::PaxosLogEntry is entered into the uni::paxos::PaxosLog. This includes
   * both writes as well as reads (which updates the last access time).
   */
  std::function<void(uni::paxos::index_t, proto::paxos::PaxosLogEntry)> get_paxos_callback();

  /**
   * @brief generated a pretty printed debug string (JSON-like indentation).
   */
  std::string debug_string() const;

 private:
  std::map<std::string, MultiVersionEntry> _mvkvs;
};

} // namespace slave
} // namespace uni


#endif // UNI_SLAVE_KVSTORE_H
