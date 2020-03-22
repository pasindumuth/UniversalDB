#ifndef UNI_SERVER_SLAVEGROUPID_H
#define UNI_SERVER_SLAVEGROUPID_H

#include <string>

#include <boost/functional/hash.hpp>

#include <common/common.h>

namespace uni {
namespace server {

struct SlaveGroupId {
  std::string id;

  SlaveGroupId(std::string const& id);

  bool operator==(const SlaveGroupId& other) const;

  bool operator<(const SlaveGroupId& other) const;
};

} // namespace server
} // namespace uni


namespace std {

template<>
struct hash<uni::server::SlaveGroupId> {
  std::size_t operator()(const uni::server::SlaveGroupId& id) const {
    // Initial hash value
    std::size_t seed = 0;

    // Combine each private member into the hash value
    boost::hash_combine(seed, boost::hash_value(id.id));

    return seed;
  }
};

} // namespace std

#endif //UNI_SERVER_SLAVEGROUPID_H
