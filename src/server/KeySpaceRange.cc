#include "KeySpaceRange.h"

#include <utils/pbutil.h>

namespace uni {
namespace server {

bool KeySpaceRange::operator==(const KeySpaceRange& other) const {
  return
    database_id == other.database_id &&
    table_id == other.table_id &&
    start_key == other.start_key &&
    end_key == other.end_key;
}

uni::server::KeySpaceRange convert(proto::common::KeySpaceRange const& range_proto) {
  return {
    range_proto.database_id(),
    range_proto.table_id(),
    range_proto.has_start_key() ? range_proto.start_key().value() : boost::optional<std::string>(),
    range_proto.has_end_key() ? range_proto.end_key().value() : boost::optional<std::string>()
  };
}

proto::common::KeySpaceRange* convert(uni::server::KeySpaceRange const& range_object) {
  auto range_proto = new proto::common::KeySpaceRange();
  build_range(range_proto, range_object);
  return range_proto;
}

void build_range(
  proto::common::KeySpaceRange *const proto_range,
  uni::server::KeySpaceRange const& range
) {
  proto_range->set_database_id(range.database_id);
  proto_range->set_table_id(range.table_id);
  if (range.start_key) proto_range->set_allocated_start_key(uni::utils::pb::string(range.start_key.get()));
  if (range.end_key) proto_range->set_allocated_end_key(uni::utils::pb::string(range.end_key.get()));
}

} // namespace server
} // namespace uni
