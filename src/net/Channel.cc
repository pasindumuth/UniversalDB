#include "Channel.h"

#include <functional>
#include <string>

#include <common/common.h>
#include <net/EndpointId.h>

namespace uni {
namespace net {

/**
 * Registers a callback to run when data is received from the other endpoint.
 * The callback returns a boolean indicating whether to continue running that callback
 * when data is received; i.e. if false is returned, then the callback stops being run.
 */
void Channel::add_receive_callback(std::function<bool(std::string)> callback) {
  _receive_callbacks.push_back(callback);
};

void Channel::add_close_callback(std::function<void(void)> callback) {
  _close_callbacks.push_back(callback);
};

} // namespace net
} // namespace uni

