#ifndef UNI_SLAVE_TABLETID_H
#define UNI_SLAVE_TABLETID_H

#include <string>

#include <boost/functional/hash.hpp>

namespace uni {
namespace slave {

struct TabletId {
  std::string database_id;
  std::string table_id;
  std::string start_key;
  std::string end_key;

  bool operator==(const TabletId& other) const;
};

} // namespace slave
} // namespace uni

namespace std {

template<>
struct hash<uni::slave::TabletId> {
  std::size_t operator()(const uni::slave::TabletId& id) const {
    // Initial hash value
    std::size_t seed = 0;

    // Combine each private member into the hash value
    boost::hash_combine(seed, boost::hash_value(id.database_id));
    boost::hash_combine(seed, boost::hash_value(id.table_id));
    boost::hash_combine(seed, boost::hash_value(id.start_key));
    boost::hash_combine(seed, boost::hash_value(id.end_key));

    return seed;
  }
};

} // namespace std


#endif // UNI_SLAVE_TABLETID_H
