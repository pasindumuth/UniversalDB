#ifndef UNI_NET_CHANNELTESTING_H
#define UNI_NET_CHANNELTESTING_H

#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <boost/optional.hpp>

#include <async/testing/AsyncSchedulerTesting.h>
#include <common/common.h>
#include <constants/constants.h>
#include <net/Channel.h>
#include <net/endpoint_id.h>

namespace uni {
namespace net {

/**
 * This class is used during testing for simulation.
 * 
 * We design this class so that like the impl classes, there are essentially 2
 * versions of this: Read mode and Write mode.
 * 
 * TODO: make 2 distinct classes for this.
 */
class ChannelTesting
    : public uni::net::Channel {
 public:
  // We pass in the async_scheduler of the receiver Universal Slave. This class directly calls the
  // async_scheduler to deliver the message.
  ChannelTesting(
      uni::constants::Constants const& constants, // present in both modes
      std::string const& other_ip_string, // present in read mode
      std::vector<uni::net::ChannelTesting*>& nonempty_channels, // present in write mode
      boost::optional<uni::net::ChannelTesting&> other_channel); // present in write mode

  ~ChannelTesting() override {};

  uni::net::endpoint_id endpoint_id() override;

  // Add a new message to the queue
  void queue_send(std::string message) override;

  // Pops the first message in the _message_queue and runs it through the _receive_callback.
  void deliver_message();

  // Recieves message and then calls the receive callbacks
  void recieve_message(std::string message);

  // Drops the message the next message (simulating a temporary loss in connectivity).
  void drop_message();

  // Sets the connection state of the channel.
  void set_connection_state(bool connection_state);

  void add_receive_callback(std::function<bool(std::string)> callback) override;

  void add_close_callback(std::function<void(void)> callback) override;

 private:
  uni::constants::Constants const& _constants;
  // The other ip address
  std::string const _other_ip_string;
  // FIFO queue which contains messages at the sender that are yet to be
  // received and processed by the receiver.
  std::queue<std::string> _message_queue;
  // This channel is responsible for maintaining its place in this vectors when
  // a call to queue_send and deliver_message is made.
  std::vector<uni::net::ChannelTesting*>& _nonempty_channels;
  // As long as the connection state is false, calling deliver_message and
  // drop_message will simply drop the message. This helps simulate long term
  // or permanent connection failures.
  bool _connection_state;

  // Other stuff
  std::vector<std::function<bool(std::string)>> _receive_callbacks;
  std::vector<std::function<void(void)>> _close_callbacks;
  boost::optional<uni::net::ChannelTesting&> _other_channel;

  // After the channel has lost a message, this function maintains the channels
  // inclusion in _nonempty_channels, removing it if the Channel becomes empty.
  void check_empty();
};

} // namespace net
} // namespace uni


#endif // UNI_NET_CHANNELTESTING_H
