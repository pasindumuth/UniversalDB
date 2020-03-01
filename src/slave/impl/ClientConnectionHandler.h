#ifndef UNI_SLAVE_CLIENTCONNECTIONHANDLER
#define UNI_SLAVE_CLIENTCONNECTIONHANDLER

#include <unordered_map>

#include <boost/asio.hpp>

#include <async/AsyncScheduler.h>
#include <common/common.h>
#include <net/Channel.h>
#include <net/ConnectionsIn.h>

namespace uni {
namespace slave {

// Responsible for scheduling accept handler and data receive handlers for
// client connections.
class ClientConnectionHandler {
 public:
  ClientConnectionHandler(
      uni::async::AsyncScheduler& scheduler,
      boost::asio::ip::tcp::acceptor& acceptor,
      uni::net::ConnectionsIn& connections_in);

  // This method is primarily used by the background thread to listen to accept new connections
  // from the acceptor, add callbacks to send their data to the AsyncScheduler when they receive
  // data, and keep track of those connections until they close.
  void async_accept();

 private:
  uni::async::AsyncScheduler& _scheduler;
  boost::asio::ip::tcp::acceptor& _acceptor;
  uni::net::ConnectionsIn& _connections_in;
};

} // slave
} // uni


#endif // UNI_SLAVE_CLIENTCONNECTIONHANDLER