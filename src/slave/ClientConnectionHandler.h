#ifndef UNI_SLAVE_CLIENTCONNECTIONHANDLER
#define UNI_SLAVE_CLIENTCONNECTIONHANDLER

#include <memory>
#include <mutex>
#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/optional.hpp>

#include <async/AsyncScheduler.h>
#include <common/common.h>
#include <net/Channel.h>

namespace uni {
namespace slave {

// Responsible for scheduling accept handler and data receive handlers for
// client connections.
class ClientConnectionHandler {
 public:
  ClientConnectionHandler(
      uni::async::AsyncScheduler& scheduler,
      boost::asio::ip::tcp::acceptor& acceptor);

  // This method is primarily used by the background thread to listen to accept new connections
  // from the acceptor, add callbacks to send their data to the AsyncScheduler when they receive
  // data, and keep track of those connections until they close.
  void async_accept();

  // This method is primary used by the server thread to get the Channel object from it's endpoint_id
  // object. This is useful if we have to send data out on that channel.
  boost::optional<std::shared_ptr<uni::net::Channel>> get_channel(uni::net::endpoint_id endpoint_id);

 private:
  uni::async::AsyncScheduler& _scheduler;
  boost::asio::ip::tcp::acceptor& _acceptor;
  std::mutex _channel_lock;
  std::unordered_map<uni::net::endpoint_id, std::shared_ptr<uni::net::Channel>> _channels;
};

} // slave
} // uni


#endif // UNI_SLAVE_CLIENTCONNECTIONHANDLER
