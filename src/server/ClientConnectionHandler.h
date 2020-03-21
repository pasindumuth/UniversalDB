#ifndef UNI_SERVER_CLIENTCONNECTIONHANDLER
#define UNI_SERVER_CLIENTCONNECTIONHANDLER

#include <unordered_map>

#include <boost/asio.hpp>

#include <async/AsyncScheduler.h>
#include <common/common.h>
#include <net/Channel.h>
#include <net/Connections.h>

namespace uni {
namespace server {

// Responsible for scheduling accept handler and data receive handlers for
// client connections.
class ClientConnectionHandler {
 public:
  ClientConnectionHandler(
      uni::async::AsyncScheduler& scheduler,
      boost::asio::ip::tcp::acceptor& acceptor,
      uni::net::Connections& connections);

  // This method is primarily used by the background thread to listen to accept new connections
  // from the acceptor, add callbacks to send their data to the AsyncScheduler when they receive
  // data, and keep track of those connections until they close.
  void async_accept();

 private:
  uni::async::AsyncScheduler& _scheduler;
  boost::asio::ip::tcp::acceptor& _acceptor;
  uni::net::Connections& _connections;
};

} // server
} // uni


#endif // UNI_SERVER_CLIENTCONNECTIONHANDLER
