#ifndef UNI_SERVER_KEYSPACERANGE_H
#define UNI_SERVER_KEYSPACERANGE_H

#include <string>

#include <boost/functional/hash.hpp>
#include <boost/optional.hpp>

#include <proto/common.pb.h>

namespace uni {
namespace server {

struct KeySpaceRange {
  std::string database_id;
  std::string table_id;
  boost::optional<std::string> start_key;
  boost::optional<std::string> end_key;

  bool operator==(const KeySpaceRange& other) const;
};

// A function template that can take in any type of paxos message with
// the methods: database_id(), table_id(), and key(), and check if those
// values fall within the range of the KeySpaceRange.
template<typename T>
bool within_range(
  uni::server::KeySpaceRange const& range,
  T const& message
) {
  if (range.database_id != message.database_id()) return false;
  if (range.table_id != message.table_id()) return false;
  if (range.start_key != boost::none && range.start_key.get() > message.key()) return false;
  if (range.end_key != boost::none && range.end_key.get() <= message.key()) return false;
  return true;
}

// Converts a KeySpaceRange proto into a KeySpaceRnage object
uni::server::KeySpaceRange convert(proto::common::KeySpaceRange const& range_proto);

// Converts a KeySpaceRange object into a KeySpaceRnage proto
proto::common::KeySpaceRange* convert(uni::server::KeySpaceRange const& range_object);

// Populates the range_proto with the contents of the range_object
void build_range(
  proto::common::KeySpaceRange *const range_proto,
  uni::server::KeySpaceRange const& range_object
);

} // namespace server
} // namespace uni

namespace std {

template<>
struct hash<uni::server::KeySpaceRange> {
  std::size_t operator()(const uni::server::KeySpaceRange& id) const {
    // Initial hash value
    std::size_t seed = 0;

    // Combine each private member into the hash value
    boost::hash_combine(seed, boost::hash_value(id.database_id));
    boost::hash_combine(seed, boost::hash_value(id.table_id));
    if (id.start_key) boost::hash_combine(seed, boost::hash_value(id.start_key.get()));
    if (id.end_key) boost::hash_combine(seed, boost::hash_value(id.end_key.get()));

    return seed;
  }
};

} // namespace std

#endif // UNI_SERVER_KEYSPACERANGE_H
