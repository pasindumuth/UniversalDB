#include <boost/asio.hpp>

#include <async/impl/AsyncSchedulerImpl.h>
#include <async/impl/TimerAsyncSchedulerImpl.h>
#include <common/common.h>
#include <logging/log.h>
#include <master/impl/ProductionContext.h>
#include <net/Connections.h>
#include <net/SelfChannel.h>
#include <net/impl/ChannelImpl.h>
#include <server/ConnectionHandler.h>
#include <utils.h>

using boost::asio::ip::tcp;

int main(int argc, char* argv[]) {
  auto ips = parse_args(argc, argv);

  // Initialize constants
  auto const constants = initialize_constants();
  auto master_ips = std::vector<std::string>(ips.begin(), ips.end() - constants.num_slave_servers);
  auto slave_ips = std::vector<std::string>(ips.end() - constants.num_slave_servers, ips.end());
  uni::logging::get_log_level() = uni::logging::Level::TRACE1;
  LOG(uni::logging::Level::INFO, "Starting main server on: " + master_ips[0] + ":" + std::to_string(constants.master_port))

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

  // Schedule main acceptor
  auto connections = uni::net::Connections(server_async_scheduler);
  auto main_acceptor = tcp::acceptor(background_io_context, tcp::endpoint(tcp::v4(), constants.master_port));
  auto server_connection_handler = uni::server::ConnectionHandler(connections, main_acceptor);

  // Timer
  auto timer_scheduler = uni::async::TimerAsyncSchedulerImpl(background_io_context);

  server_connection_handler.async_accept();

  for (auto i = 1; i < master_ips.size(); i++) {
    auto endpoint = boost::asio::ip::tcp::endpoint(
      boost::asio::ip::address(boost::asio::ip::make_address_v4(master_ips[i])),
      constants.master_port);
    auto socket = tcp::socket(background_io_context);
    socket.connect(endpoint);
    auto channel = std::make_unique<uni::net::ChannelImpl>(std::move(socket));
    connections.add_channel(std::move(channel));
    LOG(uni::logging::Level::INFO, "Connected to master node: " + master_ips[i]);
  }
  auto channel = std::make_unique<uni::net::SelfChannel>(master_ips[0]);
  connections.add_channel(std::move(channel));

  auto client_acceptor = tcp::acceptor(background_io_context, tcp::endpoint(tcp::v4(), constants.client_port));
  auto client_connections = uni::net::Connections(server_async_scheduler);
  auto client_connection_handler = uni::server::ConnectionHandler(client_connections, client_acceptor);
  
  // Slave connections
  auto slave_connections = uni::net::Connections(server_async_scheduler);
  for (auto const& slave_ip: slave_ips) {
    auto endpoint = boost::asio::ip::tcp::endpoint(
      boost::asio::ip::address(boost::asio::ip::make_address_v4(slave_ip)),
      constants.master_port);
    auto socket = tcp::socket(background_io_context);
    socket.connect(endpoint);
    auto channel = std::make_unique<uni::net::ChannelImpl>(std::move(socket));
    slave_connections.add_channel(std::move(channel));
    LOG(uni::logging::Level::INFO, "Connected to slave node: " + slave_ip);
  }

  // Compute config_endpoints
  auto config_endpoints = std::vector<uni::net::EndpointId>{};
  for (auto i = 0; i < constants.num_master_servers; i++) {
    config_endpoints.push_back({"172.19.0." + std::to_string(20 + i), 0});
  }

  // Compute slave_endpoints
  auto slave_endpoints = std::vector<uni::net::EndpointId>{};
  for (auto i = 0; i < constants.num_slave_servers; i++) {
    slave_endpoints.push_back({"172.19.0." + std::to_string(10 + i), 0});
  }

  auto production_context = uni::master::ProductionContext(
    background_io_context,
    constants,
    client_connections,
    slave_connections,
    connections,
    server_async_scheduler,
    config_endpoints,
    slave_endpoints);

  client_connection_handler.async_accept();

  LOG(uni::logging::Level::INFO, "Setup finished")
  auto server_work_guard = boost::asio::make_work_guard(server_io_context);
  server_io_context.run();
}
