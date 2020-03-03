#ifndef UNI_SLAVE_SERVERCONNECTIONHANDLER
#define UNI_SLAVE_SERVERCONNECTIONHANDLER

#include <boost/asio.hpp>

#include <common/common.h>
#include <constants/constants.h>
#include <net/Connections.h>

namespace uni {
namespace slave {

// Responsible for scheduling accept handler and data receive handlers for
// other server connections.
class ServerConnectionHandler {
 public:
  ServerConnectionHandler(
      uni::constants::Constants const& constants,
      uni::net::Connections& connections,
      boost::asio::ip::tcp::acceptor& acceptor,
      boost::asio::io_context& io_context);

  void async_accept();

 private:
  uni::constants::Constants const& _constants;
  uni::net::Connections& _connections;
  boost::asio::ip::tcp::acceptor& _acceptor;
  boost::asio::io_context& _io_context;
};

} // slave
} // uni


#endif // UNI_SLAVE_SERVERCONNECTIONHANDLER
