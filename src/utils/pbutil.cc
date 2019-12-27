#include "pbutil.h"

namespace uni {
namespace utils {
namespace pb {

google::protobuf::StringValue* string(std::string const& value) {
  auto pb = new google::protobuf::StringValue();
  pb->set_value(value);
  return pb;
}

google::protobuf::StringValue* string(google::protobuf::StringValue const& value) {
  return new google::protobuf::StringValue(value);
}

google::protobuf::UInt64Value* uint64(int64_t const& value) {
  auto pb = new google::protobuf::UInt64Value();
  pb->set_value(value);
  return pb;
}

google::protobuf::UInt64Value* uint64(google::protobuf::UInt64Value const& value) {
  return new google::protobuf::UInt64Value(value);
}

} // namespace pb
} // namespace utils
} // namespace uni
