#include "SelfChannel.h"

namespace uni {
namespace net {

SelfChannel::SelfChannel()
  : listening(false) {}

uni::net::endpoint_id SelfChannel::endpoint_id() {
  return {"localhost", 0};
}

void SelfChannel::queue_send(std::string message) {
  if (listening) {
    // Recall that Channel.h requires the user to have called start_listening
    // before we can ever start running recieve callbacks.
    for (auto const& recieve_callback : _receive_callbacks) {
      recieve_callback(message);
    }
  }
}

void SelfChannel::start_listening() {
  listening = true;
}

} // namespace net
} // namespace uni
