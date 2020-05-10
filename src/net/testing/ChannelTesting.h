#ifndef UNI_NET_CHANNELTESTING_H
#define UNI_NET_CHANNELTESTING_H

#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

#include <boost/optional.hpp>

#include <async/testing/AsyncSchedulerTesting.h>
#include <common/common.h>
#include <net/Channel.h>
#include <net/EndpointId.h>

namespace uni {
namespace net {

/**
 * This class is used during testing for simulation.
 * 
 * The other_channel and this channel form a pair, holding each other as
 * the other channel. Messages sent through queue_send goes through to the 
 * other channel and runs through their receive callbacks. The nonempty_channels
 * includes this channel if _message_queue is non-empty. If the other channel has
 * start_listening() not already called, then it 
 * 
 * Visualize this channel on the left, an arrow to the other channel on the right, 
 * nonempty_channels above filling up with this channel if the message_queue,
 * visualized beneath the line, is non-empty. The connection_state of this channels creates
 * a leak in the arrow on the left, and if start_listening() isn't called on the other channel,
 * there is a leak in the arrow on the right.
 * 
 * Properties that we observe: nonempty_channels contains this channel if message_queue
 * is non-empty. If either start_listening and or connection_state isn't set, then trying to
 * send a message will poll message_queue, but it won't go to the other side.
 */
class ChannelTesting
    : public uni::net::Channel {
 public:
  // We pass in the async_scheduler of the receiver Universal Slave. This class directly calls the
  // async_scheduler to deliver the message.
  ChannelTesting(
    std::string const& other_ip_string,
    std::vector<uni::net::ChannelTesting*>& nonempty_channels);

  ~ChannelTesting() {};

  void set_other_end(uni::net::ChannelTesting* other_channel);

  uni::net::EndpointId endpoint_id() override;

  // Adds the message to _message_queue.
  void queue_send(std::string message) override;

  void start_listening() override;

  // Pops the first message in the _message_queue and runs it through the other
  // channels recieve_message method. This will only be called when _message_queue is nonempty. 
  void deliver_message();

  // Recieves message and then calls the receive callbacks
  void recieve_message(std::string message);

  // Drops the message the next message (simulating a temporary loss in connectivity).
  void drop_message();

  // Sets the connection state of the channel.
  void set_connection_state(bool connection_state);

 private:
  // The other ip address
  std::string const _other_ip_string;
  // FIFO queue which contains messages at the sender that are yet to be
  // received and processed by the receiver.
  std::queue<std::string> _message_queue;
  // This channel is responsible for maintaining its place in this vectors when
  // a call to queue_send and deliver_message is made.
  std::vector<uni::net::ChannelTesting*>& _nonempty_channels;
  // The other end.
  uni::net::ChannelTesting* _other_channel;
  // As long as the connection state is false, calling deliver_message and
  // drop_message will simply drop the message. This helps simulate long term
  // or permanent connection failures.
  bool _connection_state;
  bool _listening;

  // After the channel has lost a message, this function maintains the channels
  // inclusion in _nonempty_channels, removing it if the Channel becomes empty.
  void check_empty();
};

} // namespace net
} // namespace uni


#endif // UNI_NET_CHANNELTESTING_H
