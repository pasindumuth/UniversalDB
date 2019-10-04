#ifndef UNI_NET_CHANNELTESTING_H
#define UNI_NET_CHANNELTESTING_H

#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

#include <boost/asio.hpp>

#include <async/testing/AsyncSchedulerTesting.h>
#include <constants/constants.h>
#include <net/Channel.h>
#include <net/endpoint_id.h>

namespace uni {
namespace net {

class ChannelTesting
    : public uni::net::Channel {
 public:
  // We pass in the async_scheduler of the receiver Universal Slave. This class directly calls the
  // async_scheduler to deliver the message.
  ChannelTesting(
      uni::constants::Constants const& constants,
      uni::async::AsyncSchedulerTesting& async_scheduler,
      std::string const& sender_ip_string,
      std::string const& receiver_ip_string,
      std::vector<uni::net::ChannelTesting*>& nonempty_channels);

  uni::net::endpoint_id endpoint_id() override;

  // Add a new message to the queue
  void queue_send(std::string message) override;

  // Pops the first message in the _message_queue and runs it through the _recieve_callback.
  void deliver_message();

 private:
  uni::constants::Constants const& _constants;
  uni::async::AsyncSchedulerTesting& _async_scheduler;
  // We need both the sender and receiver to...
  std::string const _sender_ip_string;
  std::string const _receiver_ip_string;
  // FIFO queue which contains messages at the sender that are yet to be
  // recieved and processed by the reciever.
  std::queue<std::string> _message_queue;
  // This channel is responsible for maintaining its place in this vectors when
  // a call to queue_send and deliver_message is made.
  std::vector<uni::net::ChannelTesting*>& _nonempty_channels;
};

} // namespace net
} // namespace uni


#endif // UNI_NET_CHANNELTESTING_H
