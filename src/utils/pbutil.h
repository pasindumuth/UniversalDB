#ifndef UNI_UTILS_PB_H
#define UNI_UTILS_PB_H

#include <google/protobuf/wrappers.pb.h>

namespace uni {
namespace utils {
namespace pb {

google::protobuf::StringValue* string(std::string const& value);

google::protobuf::StringValue* string(google::protobuf::StringValue const& value);

google::protobuf::UInt64Value* uint64(long const& value);

google::protobuf::UInt64Value* uint64(google::protobuf::UInt64Value const& value);

} // namespace pb
} // namespace utils
} // namespace uni

#endif // UNI_UTILS_PB_H
