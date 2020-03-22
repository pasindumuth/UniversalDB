#include "SlaveGroupId.h"

namespace uni {
namespace server {

SlaveGroupId::SlaveGroupId(std::string const& id)
  : id(id) {}

bool SlaveGroupId::operator==(const SlaveGroupId& other) const {
  return id == other.id;
}

bool SlaveGroupId::operator<(const SlaveGroupId& other) const {
  return id < other.id;
}

} // namespace server
} // namespace uni
