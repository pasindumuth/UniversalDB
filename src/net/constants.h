#ifndef UNI_NET_CONSTANTS_H
#define UNI_NET_CONSTANTS_H

#include <common/common.h>

namespace uni {
namespace net {
namespace constants {

#define MAX_MESSAGES_QUEUE_SIZE 10000

#define INT_TO_STR(i) std::string((char*) &(i), 4)

} // namespace constants
} // namespace net
} // namespace uni


#endif // UNI_NET_CONSTANTS_H
