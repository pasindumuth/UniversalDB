#ifndef UNI_SERVER_KEYSPACERANGE_H
#define UNI_SERVER_KEYSPACERANGE_H

#include <string>

#include <boost/functional/hash.hpp>
#include <boost/optional.hpp>

namespace uni {
namespace server {

struct KeySpaceRange {
  std::string database_id;
  std::string table_id;
  boost::optional<std::string> start_key;
  boost::optional<std::string> end_key;

  bool operator==(const KeySpaceRange& other) const;
};

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
