#include <boost/asio.hpp>

#include <async/impl/AsyncSchedulerImpl.h>
#include <async/impl/TimerAsyncSchedulerImpl.h>
#include <common/common.h>
#include <constants/constants.h>
#include <net/Connections.h>
#include <net/SelfChannel.h>
#include <net/impl/ChannelImpl.h>
#include <server/ConnectionHandler.h>
#include <slave/impl/ProductionContext.h>
#include <utils.h>

using boost::asio::ip::tcp;

int main(int argc, char* argv[]) {
  auto ips = parse_args(argc, argv);

  // Initialize constants
  auto const constants = initialize_constants();
  uni::logging::get_log_level() = uni::logging::Level::TRACE1;
  LOG(uni::logging::Level::INFO, "Starting main server on: " + ips[0] + ":" + std::to_string(constants.slave_port))

  // Initialize io_context for background thread (for managing the network
  // and dispatching requests to the server thread).
  auto background_io_context = boost::asio::io_context();
  auto background_work_guard = boost::asio::make_work_guard(background_io_context);
  auto network_thread = std::thread([&background_io_context](){
    background_io_context.run();
  });

  // Initialize io_context for server thread (for processing requests and
  // dispatching responses to the background thread).
  auto server_io_context = boost::asio::io_context();
  auto server_async_scheduler = uni::async::AsyncSchedulerImpl(server_io_context);

  // Setup slave acceptor.
  auto slave_acceptor = tcp::acceptor(background_io_context, tcp::endpoint(tcp::v4(), constants.slave_port));
  auto slave_connections = uni::net::Connections(server_async_scheduler);
  auto slave_connection_handler = uni::server::ConnectionHandler(slave_connections, slave_acceptor);
  slave_connection_handler.async_accept();

  // Setup client acceptor.
  auto client_acceptor = tcp::acceptor(background_io_context, tcp::endpoint(tcp::v4(), constants.client_port));
  auto client_connections = uni::net::Connections(server_async_scheduler);
  auto client_connection_handler = uni::server::ConnectionHandler(client_connections, client_acceptor);
  client_connection_handler.async_accept();

  // Setup master acceptor.
  auto master_acceptor = tcp::acceptor(background_io_context, tcp::endpoint(tcp::v4(), constants.master_port));
  auto master_connections = uni::net::Connections(server_async_scheduler);
  auto master_connection_handler = uni::server::ConnectionHandler(master_connections, master_acceptor);
  master_connection_handler.async_accept();

  for (auto i = 1; i < ips.size(); i++) {
    auto endpoint = boost::asio::ip::tcp::endpoint(
      boost::asio::ip::address(boost::asio::ip::make_address_v4(ips[i])),
      constants.slave_port);
    auto socket = tcp::socket(background_io_context);
    socket.connect(endpoint);
    auto channel = std::make_unique<uni::net::ChannelImpl>(std::move(socket));
    slave_connections.add_channel(std::move(channel));
    LOG(uni::logging::Level::INFO, "Connected to slave node: " + ips[i]);
  }
  auto self_channel = std::make_unique<uni::net::SelfChannel>(ips[0]);
  auto ip_string = self_channel->endpoint_id().ip_string;
  slave_connections.add_channel(std::move(self_channel));

  // Compute config_endpoints
  auto config_endpoints = std::vector<uni::net::EndpointId>{};
  for (auto i = 0; i < constants.num_slave_servers; i++) {
    config_endpoints.push_back({"172.19.0." + std::to_string(10 + i), 0});
  }

  auto production_context = uni::slave::ProductionContext(
    background_io_context,
    constants,
    client_connections,
    master_connections,
    slave_connections,
    server_async_scheduler,
    config_endpoints,
    ip_string);

  LOG(uni::logging::Level::INFO, "Setup finished")
  auto server_work_guard = boost::asio::make_work_guard(server_io_context);
  server_io_context.run();
}
