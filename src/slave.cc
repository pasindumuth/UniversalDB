#include <chrono>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

#include <async/impl/AsyncSchedulerImpl.h>
#include <async/impl/TimerAsyncSchedulerImpl.h>
#include <common/common.h>
#include <constants/constants.h>
#include <net/Connections.h>
#include <net/IncomingMessage.h>
#include <net/SelfChannel.h>
#include <net/impl/ChannelImpl.h>
#include <proto/message.pb.h>
#include <proto/slave.pb.h>
#include <paxos/MultiPaxosHandler.h>
#include <paxos/PaxosLog.h>
#include <paxos/PaxosTypes.h>
#include <paxos/SinglePaxosHandler.h>
#include <slave/impl/ClientConnectionHandler.h>
#include <slave/HeartbeatTracker.h>
#include <slave/FailureDetector.h>
#include <slave/LogSyncer.h>
#include <slave/impl/ProductionContext.h>
#include <slave/ProposerQueue.h>
#include <slave/impl/ServerConnectionHandler.h>
#include <slave/SlaveIncomingMessageHandler.h>
#include <utils.h>
#include <utils/pbutil.h>

using boost::asio::ip::tcp;

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
  uni::logging::get_log_level() = uni::logging::Level::TRACE1;
  LOG(uni::logging::Level::INFO, "Starting main server on: " + main_serving_hostname + ":" + std::to_string(constants.slave_port))

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
  auto main_acceptor = tcp::acceptor(background_io_context, tcp::endpoint(tcp::v4(), constants.slave_port));
  auto server_connection_handler = uni::slave::ServerConnectionHandler(constants, connections, main_acceptor, background_io_context);
  auto resolver = tcp::resolver(background_io_context);

  // Timer
  auto timer_scheduler = uni::async::TimerAsyncSchedulerImpl(background_io_context);

  // Wait for a list of all slave nodes from the master
  server_connection_handler.async_accept();

  for (auto i = 1; i < hostnames.size(); i++) {
    auto endpoints = resolver.resolve(hostnames[i], std::to_string(constants.slave_port));
    auto socket = tcp::socket(background_io_context);
    boost::asio::connect(socket, endpoints);
    connections.add_channel(std::make_unique<uni::net::ChannelImpl>(std::move(socket)));
  }
  connections.add_channel(std::make_unique<uni::net::SelfChannel>());

  auto client_acceptor = tcp::acceptor(background_io_context, tcp::endpoint(tcp::v4(), constants.client_port));
  auto client_connections = uni::net::Connections(server_async_scheduler);
  auto client_connection_handler = uni::slave::ClientConnectionHandler(server_async_scheduler, client_acceptor, client_connections);
  
  auto production_context = uni::slave::ProductionContext(
    background_io_context,
    constants,
    client_connections,
    connections);
  server_async_scheduler.set_callback([&production_context](uni::net::IncomingMessage message){
    production_context.slave_handler.handle(message);
  });

  client_connection_handler.async_accept();

  LOG(uni::logging::Level::INFO, "Setup finished")
  auto server_work_guard = boost::asio::make_work_guard(server_io_context);
  server_io_context.run();
}
