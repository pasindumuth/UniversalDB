#ifndef UNI_SERVER_CONNECTIONHANDLER_H
#define UNI_SERVER_CONNECTIONHANDLER_H

#include <boost/asio.hpp>

#include <common/common.h>
#include <constants/constants.h>
#include <net/Connections.h>

namespace uni {
namespace server {

// Responsible for scheduling accept handler and data receive handlers for
// other server connections.
class ConnectionHandler {
 public:
  ConnectionHandler(
    uni::net::Connections& connections,
    boost::asio::ip::tcp::acceptor& acceptor);

  void async_accept();

 private:
  uni::net::Connections& _connections;
  boost::asio::ip::tcp::acceptor& _acceptor;
};

} // server
} // uni


#endif // UNI_SERVER_CONNECTIONHANDLER_H
