#include "TabletId.h"

namespace uni {
namespace slave {

bool TabletId::operator==(const TabletId& other) const {
  return
    database_id == other.database_id &&
    table_id == other.table_id &&
    start_key == other.start_key &&
    end_key == other.end_key;
}

} // namespace slave
} // namespace uni
