#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

#include <async/impl/AsyncSchedulerImpl.h>
#include <constants/constants.h>
#include <logging/log.h>
#include <net/ConnectionsIn.h>
#include <net/ConnectionsOut.h>
#include <net/endpoint_id.h>
#include <net/IncomingMessage.h>
#include <net/impl/ChannelImpl.h>
#include <paxos/MultiPaxosHandler.h>
#include <paxos/PaxosLog.h>
#include <paxos/PaxosTypes.h>
#include <paxos/SinglePaxosHandler.h>
#include <slave/ClientConnectionHandler.h>
#include <slave/ClientRequestHandler.h>
#include <slave/IncomingMessageHandler.h>
#include <utils.h>

using boost::asio::ip::tcp;

// Responsible for scheduling accept handler and data recieve handlers for
// other server connections.
class ServerConnectionHandler {
 public:
  ServerConnectionHandler(
      uni::constants::Constants const& constants,
      uni::net::ConnectionsIn& connections_in,
      uni::net::ConnectionsOut& connections_out,
      tcp::acceptor& acceptor,
      boost::asio::io_context& io_context)
      : _constants(constants),
        _connections_in(connections_in),
        _connections_out(connections_out),
        _acceptor(acceptor),
        _io_context(io_context) {}

  void async_accept() {
    _acceptor.async_accept([this](const boost::system::error_code &ec, tcp::socket socket) {
      if (!ec) {
        auto const& ip_string = socket.remote_endpoint().address().to_string();
        auto const& port = _constants.slave_port;
        auto endpoint_id = uni::net::endpoint_id(ip_string, port);
        if (!_connections_out.has(endpoint_id)) {
          // Initiate reverse connection
          // Copy the address, since the socket is destroyed later.
          auto address = boost::asio::ip::address(socket.remote_endpoint().address());
          auto endpoint = tcp::endpoint(address, port);
          auto remote_socket = std::make_shared<tcp::socket>(_io_context);
          boost::asio::async_connect(*remote_socket, std::vector<tcp::endpoint>{endpoint},
              [this, remote_socket](const boost::system::error_code& ec, const tcp::endpoint& endpoint) {
                if (!ec) {
                  LOG(uni::logging::Level::DEBUG, "Reverse connection to " + remote_socket->remote_endpoint().address().to_string() + " made")
                  _connections_out.add_channel(std::make_shared<uni::net::ChannelImpl>(std::move(*remote_socket)));
                  LOG(uni::logging::Level::DEBUG, "Did we return from this??")
                }
              });
        }

        LOG(uni::logging::Level::DEBUG, "forward connection to " + socket.remote_endpoint().address().to_string() + " made")
        _connections_in.add_channel(std::make_shared<uni::net::ChannelImpl>(std::move(socket)));
        async_accept();
      } else {
        std::cout << ec.message() << std::endl;
      }
    });
  }

 private:
  uni::constants::Constants const& _constants;
  uni::net::ConnectionsIn& _connections_in;
  uni::net::ConnectionsOut& _connections_out;
  tcp::acceptor& _acceptor;
  boost::asio::io_context& _io_context;
};

/**
 * Arguments:
 * hostname - hostname this server is running on
 * port - port this server is running on
 */
int main(int argc, char* argv[]) {
  auto hostnames = parse_hostnames(argc, argv);
  auto main_serving_hostname = hostnames[0];

  // Initialize constants
  auto const constants = initialize_constants();
  LOG(uni::logging::Level::DEBUG, "Starting main server on: " + main_serving_hostname + ":" + std::to_string(constants.slave_port))

  // Initialize io_context for background thread (for managing the network
  // and dispatching requests to the server thread).
  auto background_io_context = boost::asio::io_context();
  auto work = boost::asio::make_work_guard(background_io_context);
  auto network_thread = std::thread([&background_io_context](){
    background_io_context.run();
  });

  // Initialize io_context for server thread (for processing requests and
  // dispatching responses to the background thread).
  auto server_io_context = boost::asio::io_context();
  auto server_async_scheduler = uni::async::AsyncSchedulerImpl(server_io_context);

  // Schedule main acceptor
  auto connections_in = uni::net::ConnectionsIn(server_async_scheduler);
  auto connections_out = uni::net::ConnectionsOut(constants);
  auto main_acceptor = tcp::acceptor(background_io_context, tcp::endpoint(tcp::v4(), constants.slave_port));
  auto server_connection_handler = ServerConnectionHandler(constants, connections_in, connections_out, main_acceptor, background_io_context);
  auto resolver = tcp::resolver(background_io_context);

  // Create self connection
  main_acceptor.async_accept([&connections_in](const boost::system::error_code &ec, tcp::socket socket) {
    LOG(uni::logging::Level::DEBUG, "Self connection acceptor has been invoked")
    connections_in.add_channel(std::make_shared<uni::net::ChannelImpl>(std::move(socket)));
  });

  auto self_endpoints = resolver.resolve(main_serving_hostname, std::to_string(constants.slave_port));
  auto self_socket = tcp::socket(background_io_context);
  boost::asio::connect(self_socket, self_endpoints);
  connections_out.add_channel(std::make_shared<uni::net::ChannelImpl>(std::move(self_socket)));

  // Schedule connections to initial remote endpoints
  for(int i = 1; i < hostnames.size(); i++) {
    auto endpoints = resolver.resolve(hostnames[i], std::to_string(constants.slave_port));
    auto socket = std::make_shared<tcp::socket>(background_io_context);
    boost::asio::async_connect(*socket, endpoints,
        [&connections_out, socket](const boost::system::error_code& ec, const tcp::endpoint& endpoint) {
      if (!ec) {
        LOG(uni::logging::Level::DEBUG, "first connection to " + socket->remote_endpoint().address().to_string() + " made")
        connections_out.add_channel(std::make_shared<uni::net::ChannelImpl>(std::move(*socket)));
      }
    });
  }

  // Schedule client acceptor
  auto paxos_log = uni::paxos::PaxosLog();
  auto paxos_instance_provider = [&constants, &connections_out, &paxos_log](uni::paxos::index_t index) {
    return uni::paxos::SinglePaxosHandler(constants, connections_out, paxos_log, index);
  };
  auto multipaxos_handler = uni::paxos::MultiPaxosHandler(paxos_log, paxos_instance_provider);
  auto client_acceptor = tcp::acceptor(background_io_context, tcp::endpoint(tcp::v4(), constants.client_port));
  auto client_connection_handler = uni::slave::ClientConnectionHandler(server_async_scheduler, client_acceptor);
  auto client_request_handler = uni::slave::ClientRequestHandler(multipaxos_handler);
  auto incoming_message_handler = uni::slave::IncomingMessageHandler(client_request_handler, multipaxos_handler);
  server_async_scheduler.set_callback([&incoming_message_handler](uni::net::IncomingMessage message){
    incoming_message_handler.handle(message);
  });

  server_connection_handler.async_accept();
  client_connection_handler.async_accept();

  LOG(uni::logging::Level::DEBUG, "Setup finished")

  auto server_work = boost::asio::make_work_guard(server_io_context);
  server_io_context.run();
}
