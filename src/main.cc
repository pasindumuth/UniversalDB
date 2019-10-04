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
      std::shared_ptr<uni::net::ConnectionsIn> connections_in,
      std::shared_ptr<uni::net::ConnectionsOut> connections_out,
      std::shared_ptr<tcp::acceptor> acceptor,
      std::shared_ptr<boost::asio::io_context> io_context)
      : _constants(constants),
        _connections_in(connections_in),
        _connections_out(connections_out),
        _acceptor(acceptor),
        _io_context(io_context) {}

  void async_accept() {
    _acceptor->async_accept([this](const boost::system::error_code &ec, tcp::socket socket) {
      if (!ec) {
        auto const& ip_string = socket.remote_endpoint().address().to_string();
        auto const& port = _constants.slave_port;
        uni::net::endpoint_id endpoint_id(ip_string, port);
        if (!_connections_out->has(endpoint_id)) {
          // Initiate reverse connection
          boost::asio::ip::address address(socket.remote_endpoint().address()); // Copy the address, since the socket is destroyed later.
          tcp::endpoint endpoint(address, port);
          auto remote_socket = std::make_shared<tcp::socket>(*_io_context);
          boost::asio::async_connect(*remote_socket, std::vector<tcp::endpoint>{endpoint},
              [this, remote_socket](const boost::system::error_code& ec, const tcp::endpoint& endpoint) {
                if (!ec) {
                  std::cout << "reverse connection to " << remote_socket->remote_endpoint().address().to_string() << " made" << std::endl;
                  _connections_out->add_channel(std::make_shared<uni::net::ChannelImpl>(std::move(*remote_socket)));
                }
              });
        }

        _connections_in->add_channel(std::make_shared<uni::net::ChannelImpl>(std::move(socket)));
        async_accept();
      } else {
        std::cout << ec.message() << std::endl;
      }
    });
  }

 private:
  uni::constants::Constants const& _constants;
  std::shared_ptr<uni::net::ConnectionsIn> _connections_in;
  std::shared_ptr<uni::net::ConnectionsOut> _connections_out;
  std::shared_ptr<tcp::acceptor> _acceptor;
  std::shared_ptr<boost::asio::io_context> _io_context;
};

uni::constants::Constants initialize_constants() {
  return uni::constants::Constants(1, 1610);
}

/**
 * Arguments:
 * hostname - hostname this server is running on
 * port - port this server is running on
 */
int main(int argc, char* argv[]) {
  auto endpoints = parse_args(argc, argv);
  auto main_serving_endpoint = endpoints[0];
  auto client_serving_endpoint = endpoints[1];

  // Initialize constants
  auto const constants = initialize_constants();

  std::string main_serving_hostname(std::get<0>(main_serving_endpoint));
  int main_serving_port = std::stoi(std::get<1>(main_serving_endpoint));
  std::cout << "Starting main server on: " << main_serving_hostname << ":" << main_serving_port << std::endl;

  std::string client_serving_hostname(std::get<0>(client_serving_endpoint));
  int client_serving_port = std::stoi(std::get<1>(client_serving_endpoint));
  std::cout << "Starting client server on: " << client_serving_hostname << ":" << client_serving_port << std::endl;

  // Initialize io_context for background thread (for managing the network
  // and dispatching requests to the server thread).
  auto background_io_context = std::make_shared<boost::asio::io_context>();
  auto work = boost::asio::make_work_guard(*background_io_context);
  std::thread network_thread([background_io_context](){
    background_io_context->run();
  });

  // Initialize io_context for server thread (for processing requests and
  // dispatching responses to the background thread).
  auto server_io_context = std::make_shared<boost::asio::io_context>();
  auto server_async_scheduler = std::make_shared<uni::async::AsyncSchedulerImpl>(server_io_context);

  // Schedule main acceptor
  auto connections_in = std::make_shared<uni::net::ConnectionsIn>(server_async_scheduler);
  auto connections_out = std::make_shared<uni::net::ConnectionsOut>(constants);
  auto main_acceptor = std::make_shared<tcp::acceptor>(*background_io_context, tcp::endpoint(tcp::v4(), main_serving_port));
  auto server_connection_handler = std::make_shared<ServerConnectionHandler>(constants, connections_in, connections_out, main_acceptor, background_io_context);
  tcp::resolver resolver(*background_io_context);

  // Create self connection
  main_acceptor->async_accept([&connections_in](const boost::system::error_code &ec, tcp::socket socket) {
    connections_in->add_channel(std::make_shared<uni::net::ChannelImpl>(std::move(socket)));
  });

  tcp::resolver::results_type self_endpoints = resolver.resolve(
      std::get<0>(main_serving_endpoint), std::get<1>(main_serving_endpoint));
  tcp::socket self_socket(*background_io_context);
  boost::asio::connect(self_socket, self_endpoints);
  connections_out->add_channel(std::make_shared<uni::net::ChannelImpl>(std::move(self_socket)));

  // Schedule connections to initial remote endpoints
  for(auto it = endpoints.begin() + 2; it != endpoints.end(); ++it) {
    auto endpoint_string = *it;
    tcp::resolver::results_type endpoints = resolver.resolve(
        std::get<0>(endpoint_string), std::get<1>(endpoint_string));
    auto socket = std::make_shared<tcp::socket>(*background_io_context);
    boost::asio::async_connect(*socket, endpoints,
        [connections_out, socket](const boost::system::error_code& ec, const tcp::endpoint& endpoint) {
      if (!ec) {
        std::cout << "first connection to " << socket->remote_endpoint().address().to_string() << " made" << std::endl;
        connections_out->add_channel(std::make_shared<uni::net::ChannelImpl>(std::move(*socket)));
      }
    });
  }

  // Schedule client acceptor
  auto paxos_log = std::make_shared<uni::paxos::PaxosLog>();
  auto paxos_instance_provider = [constants, connections_out, paxos_log](uni::paxos::index_t index) {
    return uni::paxos::SinglePaxosHandler(constants, connections_out, paxos_log, index);
  };
  auto multipaxos_handler = std::make_shared<uni::paxos::MultiPaxosHandler>(paxos_log, paxos_instance_provider);
  auto client_acceptor = std::make_shared<tcp::acceptor>(*background_io_context, tcp::endpoint(tcp::v4(), client_serving_port));
  auto client_connection_handler = std::make_shared<uni::slave::ClientConnectionHandler>(server_async_scheduler, client_acceptor);
  auto client_request_handler = std::make_shared<uni::slave::ClientRequestHandler>(multipaxos_handler);
  auto incoming_message_handler = std::make_shared<uni::slave::IncomingMessageHandler>(client_request_handler, multipaxos_handler);
  server_async_scheduler->set_callback([incoming_message_handler](uni::net::IncomingMessage message){
    incoming_message_handler->handle(message);
  });

  server_connection_handler->async_accept();
  client_connection_handler->async_accept();

  auto server_work = boost::asio::make_work_guard(*server_io_context);
  server_io_context->run();
}
