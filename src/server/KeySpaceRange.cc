#include "KeySpaceRange.h"

namespace uni {
namespace server {

bool KeySpaceRange::operator==(const KeySpaceRange& other) const {
  return
    database_id == other.database_id &&
    table_id == other.table_id &&
    start_key == other.start_key &&
    end_key == other.end_key;
}

} // namespace server
} // namespace uni
