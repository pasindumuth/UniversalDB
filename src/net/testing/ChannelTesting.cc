#include "ChannelTesting.h"

#include <algorithm>
#include <iostream>
#include <vector>

#include <assert/assert.h>
#include <net/constants.h>

namespace uni {
namespace net {

ChannelTesting::ChannelTesting(
  std::string const& other_ip_string,
  std::vector<ChannelTesting*>& nonempty_channels)
  : _other_ip_string(other_ip_string),
    _nonempty_channels(nonempty_channels),
    _other_channel(nullptr),
    _connection_state(true),
    _listening(false) {}

void ChannelTesting::queue_send(std::string message) {
  if (_message_queue.size() < MAX_MESSAGES_QUEUE_SIZE) {
    _message_queue.push(message);
    if (_message_queue.size() == 1) {
      // This channel has just became nonempty, so add it to the nonempty_channels vector.
      UNIVERSAL_ASSERT_MESSAGE(
          std::find(_nonempty_channels.begin(), _nonempty_channels.end(), this) == _nonempty_channels.end(),
          "This channel should not already be in the nonempty_channels vectors, since it was indeed empty")
      _nonempty_channels.push_back(this);
    }
  }
}

// Move these 2 above queue_send.
void ChannelTesting::set_other_end(uni::net::ChannelTesting* other_channel) {
  _other_channel = other_channel;
}

uni::net::EndpointId ChannelTesting::endpoint_id() {
  return {_other_ip_string, 0};
}

void ChannelTesting::start_listening() {
  _listening = true;
}

void ChannelTesting::deliver_message() {
  UNIVERSAL_ASSERT_MESSAGE(_message_queue.size() > 0,
      "We should never be trying to deliver a message from an empty channel")
  if (!_connection_state) {
    // If the connection state is false, drop all messages.
    drop_message();
  } else {
    auto message = _message_queue.front();
    // Create the Incoming Message and dispatch it to async_scheduler.
    _other_channel->recieve_message(message);
    // Remove the message that was just processed.
    _message_queue.pop();
    check_empty();
  }
}

void ChannelTesting::recieve_message(std::string message) {
  if (_listening) {
    for (auto const& recieve_callback : _receive_callbacks) {
      recieve_callback(message);
    }
  }
}

void ChannelTesting::drop_message() {
  UNIVERSAL_ASSERT_MESSAGE(_message_queue.size() > 0,
      "We should never be trying to deliver a message from an empty channel")
  _message_queue.pop();
  check_empty();
}

void ChannelTesting::set_connection_state(bool connection_state) {
  _connection_state = connection_state;
}

void ChannelTesting::check_empty() {
  if (_message_queue.size() == 0) {
    // This channel just became empty, so remove it from the nonempty_channels vector.
    auto it = std::find(_nonempty_channels.begin(), _nonempty_channels.end(), this);
    UNIVERSAL_ASSERT_MESSAGE(it !=  _nonempty_channels.end(),
        "This channel should have been in the nonempty_channels vectors, since it was indeed nonempty")
    _nonempty_channels.erase(it);
  }
}

} // namespace net
} // namespace uni
